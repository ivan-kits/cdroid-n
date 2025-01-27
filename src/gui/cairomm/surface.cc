/* Copyright (C) 2005 The cairomm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <cairomm/surface.h>
#include <cairomm/context.h>
#include <cairomm/script.h>
#include <cairomm/private.h>
#include <cairomm/cairo_jpg.h>
#include <cairomm/bitmap.h>
#include <cdtypes.h>
#include <cdlog.h>
#include <chrono>
#include <fstream>

namespace Cairo
{

static cairo_user_data_key_t USER_DATA_KEY_WRITE_FUNC = {0};
static cairo_user_data_key_t USER_DATA_KEY_READ_FUNC = {0};

static void
free_slot(void* data)
{
  auto slot = static_cast<Surface::SlotWriteFunc*>(data);
  delete slot;
}

static Surface::SlotWriteFunc*
get_slot(cairo_surface_t* surface) {
  return static_cast<Surface::SlotWriteFunc*>(cairo_surface_get_user_data(surface,
                                                                 &USER_DATA_KEY_WRITE_FUNC));
}

static void
set_read_slot(cairo_surface_t* surface, Surface::SlotReadFunc* slot) {
  // the slot will automatically be freed by free_slot() when the underlying C
  // instance is destroyed
  cairo_surface_set_user_data(surface, &USER_DATA_KEY_READ_FUNC, slot, &free_slot);
}

static void
set_write_slot(cairo_surface_t* surface, Surface::SlotWriteFunc* slot) {
  // the slot will automatically be freed by free_slot() when the underlying C
  // instance is destroyed
  cairo_surface_set_user_data(surface, &USER_DATA_KEY_WRITE_FUNC, slot, &free_slot);
}

cairo_status_t read_func_wrapper(void* closure, unsigned char* data, unsigned int length)
{
  if (!closure)
    return CAIRO_STATUS_READ_ERROR;
  auto read_func = static_cast<Surface::SlotReadFunc*>(closure);
  return static_cast<cairo_status_t>((*read_func)(data, length));
}

cairo_status_t write_func_wrapper(void* closure, const unsigned char* data, unsigned int length)
{
  if (!closure)
    return CAIRO_STATUS_WRITE_ERROR;
  auto write_func = static_cast<Surface::SlotWriteFunc*>(closure);
  return static_cast<cairo_status_t>((*write_func)(data, length));
}

Surface::Surface(cairo_surface_t* cobject, bool has_reference)
: m_cobject(nullptr)
{
  if(has_reference)
    m_cobject = cobject;
  else
    m_cobject = cairo_surface_reference(cobject);
}

Surface::~Surface()
{
  if(m_cobject)
    cairo_surface_destroy(m_cobject);
}

void Surface::finish()
{
  cairo_surface_finish(cobj());
  check_object_status_and_throw_exception(*this);
}

const unsigned char* Surface::get_mime_data(const std::string& mime_type, unsigned long& length)
{
  const unsigned char* data = nullptr;
  cairo_surface_get_mime_data(const_cast<cobject*>(cobj()), mime_type.c_str(), &data, &length);
  check_object_status_and_throw_exception(*this);
  return data;
}


static void on_cairo_destroy(void *data)
{
  auto slot = static_cast<Surface::SlotDestroy*>(data);
  if(!slot)
    return;

  (*slot)();
  delete slot;
}

void Surface::set_mime_data(const std::string& mime_type, unsigned char* data, unsigned long length, const SlotDestroy& slot_destroy)
{
  auto copy = new SlotDestroy(slot_destroy); //Deleted when the callback is called once.
  cairo_surface_set_mime_data(const_cast<cobject*>(cobj()), mime_type.c_str(), data, length,
    &on_cairo_destroy, copy);
  check_object_status_and_throw_exception(*this);
}

void Surface::unset_mime_data(const std::string& mime_type)
{
  cairo_surface_set_mime_data(const_cast<cobject*>(cobj()), mime_type.c_str(),
    nullptr, 0, nullptr, nullptr);
  check_object_status_and_throw_exception(*this);
}

void Surface::get_font_options(FontOptions& options) const
{
  auto cfontoptions = cairo_font_options_create();
  cairo_surface_get_font_options(const_cast<cobject*>(cobj()), cfontoptions);
  options = FontOptions(cfontoptions);
  cairo_font_options_destroy(cfontoptions);
  check_object_status_and_throw_exception(*this);
}

void Surface::flush()
{
  cairo_surface_flush(cobj());
  check_object_status_and_throw_exception(*this);
}

void Surface::mark_dirty()
{
  cairo_surface_mark_dirty(cobj());
  check_object_status_and_throw_exception(*this);
}

void Surface::mark_dirty(int x, int y, int width, int height)
{
  cairo_surface_mark_dirty_rectangle(cobj(), x, y, width, height);
  check_object_status_and_throw_exception(*this);
}

void Surface::set_device_offset(double x_offset, double y_offset)
{
  cairo_surface_set_device_offset(cobj(), x_offset, y_offset);
  check_object_status_and_throw_exception(*this);
}

void Surface::get_device_offset(double& x_offset, double& y_offset) const
{
  cairo_surface_get_device_offset(const_cast<cobject*>(cobj()), &x_offset, &y_offset);
}

void Surface::set_device_scale(double x_scale,double y_scale){
  cairo_surface_set_device_scale(const_cast<cobject*>(cobj()),x_scale,y_scale);
}

void Surface::get_device_scale(double&x_scale,double&y_scale)const{
  cairo_surface_get_device_scale(const_cast<cobject*>(cobj()),&x_scale,&y_scale);
}

void Surface::set_fallback_resolution(double x_pixels_per_inch, double y_pixels_per_inch)
{
  cairo_surface_set_fallback_resolution(cobj(), x_pixels_per_inch, y_pixels_per_inch);
  check_object_status_and_throw_exception(*this);
}

void Surface::get_fallback_resolution(double& x_pixels_per_inch,
                                      double& y_pixels_per_inch) const
{
  cairo_surface_get_fallback_resolution(const_cast<cairo_surface_t*>(cobj()),
                                        &x_pixels_per_inch,
                                        &y_pixels_per_inch);
  check_object_status_and_throw_exception(*this);
}

Surface::Type Surface::get_type() const
{
  auto surface_type =
    cairo_surface_get_type(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return static_cast<Type>(surface_type);
}

Content Surface::get_content() const
{
  auto content = cairo_surface_get_content(const_cast<cairo_surface_t*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return static_cast<Content>(content);
}

void Surface::copy_page()
{
  cairo_surface_copy_page(cobj());
  check_object_status_and_throw_exception(*this);
}

void Surface::show_page()
{
  cairo_surface_show_page(cobj());
  check_object_status_and_throw_exception(*this);
}

bool Surface::has_show_text_glyphs() const
{
  bool result = cairo_surface_has_show_text_glyphs(const_cast<cairo_surface_t*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return result;
}



#ifdef CAIRO_HAS_PNG_FUNCTIONS
void Surface::write_to_png(const std::string& filename)
{
  auto status = cairo_surface_write_to_png(cobj(), filename.c_str());
  check_status_and_throw_exception(status);
}

void Surface::write_to_png_stream(const SlotWriteFunc& write_func)
{
  auto old_slot = get_slot(cobj());
  if (old_slot)
    delete old_slot;
  auto slot_copy = new SlotWriteFunc(write_func);
  set_write_slot(cobj(), slot_copy);
  auto status = cairo_surface_write_to_png_stream(cobj(),
                                                         &write_func_wrapper,
                                                         slot_copy /*closure*/);
  check_status_and_throw_exception(status);
}

void Surface::write_to_png(cairo_write_func_t write_func, void *closure)
{
  auto status = cairo_surface_write_to_png_stream(cobj(), write_func, closure);
  check_status_and_throw_exception(status);
}
#endif

void Surface::write_to_jpg(const std::string& filename){
     //auto status = cairo_surface_write_to_jpeg(cobj(),filename.c_str());
     //check_status_and_throw_exception(status);    
}

void Surface::write_to_jpg_stream(const SlotWriteFunc& write_func){
#if 0
    auto old_slot = get_slot(cobj());
    if (old_slot)
      delete old_slot;
    auto slot_copy = new SlotWriteFunc(write_func);
    set_write_slot(cobj(), slot_copy);
    auto status = cairo_surface_write_to_jpeg_stream(cobj(),
                                                         &write_func_wrapper,
                                                         slot_copy /*closure*/);
    check_status_and_throw_exception(status);
#endif
}

#if defined(ENABLE_JPEG)||defined(ENABLE_TURBOJPEG)
RefPtr<ImageSurface> ImageSurface::create_from_jpg(std::string filename){
    auto cobject = cairo_image_surface_create_from_jpeg(filename.c_str());
    check_status_and_throw_exception(cairo_surface_status(cobject));
    return RefPtr<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}

RefPtr<ImageSurface> ImageSurface::create_from_jpg_stream(const SlotReadFunc& read_func)
{
    auto slot_copy = new SlotReadFunc(read_func);
    auto cobject =  cairo_image_surface_create_from_jpeg_stream(&read_func_wrapper, slot_copy);
    check_status_and_throw_exception(cairo_surface_status(cobject));
    set_read_slot(cobject, slot_copy);
    return RefPtr<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}

RefPtr<ImageSurface> ImageSurface::create_from_jpg(cairo_read_func_t read_func, void *closure){
    auto cobject = cairo_image_surface_create_from_jpeg_stream(read_func, closure);
    check_status_and_throw_exception(cairo_surface_status(cobject));
    return RefPtr<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}

RefPtr<ImageSurface> ImageSurface::create_from_jpg(void*mem,size_t size){
    auto cobject = cairo_image_surface_create_from_jpeg_mem(mem,size);
    check_status_and_throw_exception(cairo_surface_status(cobject));
    return RefPtr<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}
#endif

static  cairo_status_t stream_read(void *closure,unsigned char *data,unsigned int length){
    std::istream*is=(std::istream*)closure;
    if(is->eof())return CAIRO_STATUS_READ_ERROR;
    is->read((char*)data,length);
    return CAIRO_STATUS_SUCCESS;
}

typedef struct FileType {
    const char *type;
    const char*identifer;
    uint8_t size;
} FileType;

#define PAK_GEN_FILE_TYPE(type, identifer)  {type, identifer, sizeof(identifer) - 1}

static const FileType FILE_TYPES[] = {
    PAK_GEN_FILE_TYPE("png", "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"),
    PAK_GEN_FILE_TYPE("jpg", "\xff\xd8\xff\xe0"),
    PAK_GEN_FILE_TYPE("jpg", "\xff\xd8\xff\xe1"),
    PAK_GEN_FILE_TYPE("bmp", "\x42\x4d"),
    PAK_GEN_FILE_TYPE("bmp", "\x4d\x42"),
    PAK_GEN_FILE_TYPE("svg", "<svg "),
    PAK_GEN_FILE_TYPE("svg", "<?xml")
};

static const char*pakGetFileType(const char*filebuf) {
    for (unsigned int i = 0; i < sizeof(FILE_TYPES)/sizeof(FileType); i++)
        if (memcmp(filebuf, FILE_TYPES[i].identifer, FILE_TYPES[i].size) == 0)
            return FILE_TYPES[i].type;
    return "";
}

using namespace std::chrono;
RefPtr<ImageSurface>ImageSurface:: create_from_stream(std::istream& stream){
    unsigned char head[8]={0};
    RefPtr<ImageSurface>img;
    steady_clock::time_point t2,t1=steady_clock::now();
    stream.read((char*)head,sizeof(head));
    const unsigned char*ftype=(const unsigned char*)pakGetFileType((const char*)head);

    LOGE_IF(!stream.good(),"stream error");
    if(!stream.good())return nullptr;

    for(int i=0;i<sizeof(head);i++)stream.unget();
    if(memcmp("jpg",ftype,3)==0){
       cairo_surface_t* cobject=nullptr;
#ifdef ENABLE_TURBOJPEG
       cobject=cairo_image_surface_create_from_turbojpeg_stdstream(stream);
#endif
#ifdef ENABLE_JPEG
       if(nullptr==cobject )cairo_image_surface_create_from_jpeg_stdstream(stream);
#endif
       img=RefPtr<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
    }else if(memcmp("png",ftype,3)==0){
        img=create_from_png(stream_read,&stream);
    }else if(memcmp("bmp",ftype,3)==0){
        Bitmap bmp;;
        bmp.ReadFromStream(stream);
        img=(RefPtr<ImageSurface>)bmp;
    }else if(memcmp("svg",ftype,3)==0){
#ifdef ENABLE_CAIROSVG
         svg_cairo_t *svg;
         unsigned int width,height;
         svg_cairo_create(&svg);
         svg_cairo_parse_chunk_begin(svg);
         while(!stream.eof()){
            char buff[512];
            stream.read(buff,sizeof(buff));
            svg_cairo_parse_chunk(svg,buff,stream.gcount());
         }
         svg_cairo_parse_chunk_end(svg);
         svg_cairo_get_size(svg,&width,&height);
         LOGD("svg.viewport=%dx%d",width,height);
         if(width>0&&height>0){
             img=ImageSurface::create(Surface::Format::ARGB32,width,height);
             RefPtr<Context>ctx=Context::create(img);
             svg_cairo_render(svg,(cairo_t*)ctx->cobj());
         }
         svg_cairo_destroy(svg);
#endif
    }
    t2=steady_clock::now();
    LOGV_IF(img.get(),"img.size=%dx%d used %.5f stype=%s",img->get_width(),img->get_height(),duration_cast<duration<double>>(t2 - t1),ftype);
    return img;
}

RefPtr<Device> Surface::get_device(){
  auto *d = cairo_surface_get_device (m_cobject);

  if (!d)
    return RefPtr<Device>();

  auto surface_type = cairo_surface_get_type(m_cobject);
  switch (surface_type)
  {
#if CAIRO_HAS_SCRIPT_SURFACE
    case CAIRO_SURFACE_TYPE_SCRIPT:
      return make_refptr_for_instance<Script>(new Script(d, true /* has reference */));
      break;
#endif
    default:
      return make_refptr_for_instance<Device>(new Device(d, true /* has reference */));
  }
}

void Surface::reference() const
{
  cairo_surface_reference(const_cast<cobject*>(cobj()));
}

void Surface::unreference() const
{
  cairo_surface_destroy(const_cast<cobject*>(cobj()));
}

RefPtr<Surface> Surface::create(const RefPtr<Surface> other, Content content, int width, int height)
{
  auto cobject = cairo_surface_create_similar(other->cobj(), (cairo_content_t)content, width, height);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<Surface>(new Surface(cobject, true /* has reference */));
}

RefPtr<Surface> Surface::create(const RefPtr<Surface>& target, double x, double y, double width, double height)
{
  auto cobject = cairo_surface_create_for_rectangle(target->cobj(), x, y, width, height);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<Surface>(new Surface(cobject, true /* has reference */));
}


ImageSurface::ImageSurface(cairo_surface_t* cobject, bool has_reference)
: Surface(cobject, has_reference)
{ }

ImageSurface::~ImageSurface()
{
  // surface is destroyed in base class
}

RefPtr<ImageSurface> ImageSurface::create(Format format, int width, int height)
{
  auto cobject = cairo_image_surface_create((cairo_format_t)format, width, height);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}

RefPtr<ImageSurface> ImageSurface::create(unsigned char* data, Format format, int width, int height, int stride)
{
  auto cobject = cairo_image_surface_create_for_data(data, (cairo_format_t)format, width, height, stride);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}

#ifdef CAIRO_HAS_PNG_FUNCTIONS

RefPtr<ImageSurface> ImageSurface::create_from_png(std::string filename)
{
  auto cobject =cairo_image_surface_create_from_png(filename.c_str());
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}

RefPtr<ImageSurface> ImageSurface::create_from_png_stream(const SlotReadFunc& read_func)
{
  auto slot_copy = new SlotReadFunc(read_func);
  auto cobject =
    cairo_image_surface_create_from_png_stream(&read_func_wrapper, slot_copy);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  set_read_slot(cobject, slot_copy);
  return make_refptr_for_instance<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}

RefPtr<ImageSurface> ImageSurface::create_from_png(cairo_read_func_t read_func, void *closure){
    auto cobject = cairo_image_surface_create_from_png_stream(read_func, closure);
    check_status_and_throw_exception(cairo_surface_status(cobject));
    return RefPtr<ImageSurface>(new ImageSurface(cobject, true /* has reference */));
}

#endif // CAIRO_HAS_PNG_FUNCTIONS

int ImageSurface::get_width() const
{
  const auto result =
    cairo_image_surface_get_width(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return result;
}

int ImageSurface::get_height() const
{
  const auto result =
    cairo_image_surface_get_height(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return result;
}

unsigned char* ImageSurface::get_data()
{
  return cairo_image_surface_get_data(cobj());
}

const unsigned char* ImageSurface::get_data() const
{
  return cairo_image_surface_get_data(const_cast<cobject*>(cobj()));
}

Surface::Format ImageSurface::get_format() const
{
  return static_cast<Format>(cairo_image_surface_get_format(const_cast<cobject*>(cobj())));
}

int ImageSurface::get_stride() const
{
  return cairo_image_surface_get_stride(const_cast<cobject*>(cobj()));
}

int ImageSurface::format_stride_for_width (Format format, int width)
{
  return cairo_format_stride_for_width(static_cast<cairo_format_t>(format), width);
}


RecordingSurface::RecordingSurface(cairo_surface_t* cobject, bool has_reference)
: Surface(cobject, has_reference)
{ }

RecordingSurface::~RecordingSurface()
{
  // surface is destroyed in base class
}

RefPtr<RecordingSurface> RecordingSurface::create(Content content)
{
  auto cobject = cairo_recording_surface_create((cairo_content_t)content, nullptr);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<RecordingSurface>(new RecordingSurface(cobject, true /* has reference */));
}

RefPtr<RecordingSurface> RecordingSurface::create(const Rectangle& extents, Content content)
{
  auto cobject = cairo_recording_surface_create((cairo_content_t)content, &extents);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<RecordingSurface>(new RecordingSurface(cobject, true /* has reference */));
}

Rectangle RecordingSurface::ink_extents() const
{
  Rectangle inked;
  cairo_recording_surface_ink_extents(const_cast<cobject*>(cobj()),
      &inked.x, &inked.y, &inked.width, &inked.height);
  check_object_status_and_throw_exception(*this);
  return inked;
}

bool RecordingSurface::get_extents(Rectangle& extents) const
{
  bool has_extents = cairo_recording_surface_get_extents(const_cast<cobject*>(cobj()),
      &extents);
  check_object_status_and_throw_exception(*this);
  return has_extents;
}


/*******************************************************************************
 * THE FOLLOWING SURFACE TYPES ARE EXPERIMENTAL AND NOT FULLY SUPPORTED
 ******************************************************************************/

#ifdef CAIRO_HAS_PDF_SURFACE

PdfSurface::PdfSurface(cairo_surface_t* cobject, bool has_reference) :
    Surface(cobject, has_reference)
{}

PdfSurface::~PdfSurface()
{
  // surface is destroyed in base class
}

RefPtr<PdfSurface> PdfSurface::create(std::string filename, double width_in_points, double height_in_points)
{
  auto cobject = cairo_pdf_surface_create(filename.c_str(), width_in_points, height_in_points);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<PdfSurface>(new PdfSurface(cobject, true /* has reference */));
}

RefPtr<PdfSurface> PdfSurface::create_for_stream(const SlotWriteFunc& write_func, double
                                                 width_in_points, double height_in_points)
{
  auto slot_copy = new SlotWriteFunc(write_func);
  auto cobject =
    cairo_pdf_surface_create_for_stream(write_func_wrapper, slot_copy,
                                        width_in_points, height_in_points);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  set_write_slot(cobject, slot_copy);
  return make_refptr_for_instance<PdfSurface>(new PdfSurface(cobject, true /* has reference */));
}

void PdfSurface::set_size(double width_in_points, double height_in_points)
{
  cairo_pdf_surface_set_size(cobj(), width_in_points, height_in_points);
  check_object_status_and_throw_exception(*this);
}

void PdfSurface::restrict_to_version(PdfVersion version)
{
  cairo_pdf_surface_restrict_to_version(cobj(), static_cast<cairo_pdf_version_t>(version));
  check_object_status_and_throw_exception(*this);
}

const std::vector<PdfVersion> PdfSurface::get_versions()
{
  cairo_pdf_version_t const *versions;
  int num_versions;
  cairo_pdf_get_versions(&versions, &num_versions);

  // Just copy the version array out into a std::vector.
  std::vector<PdfVersion> vec;
  for (int i = 0; i < num_versions; ++i)
  {
    vec.push_back(static_cast<PdfVersion>(versions[i]));
  }
  return vec;
}

std::string PdfSurface::version_to_string(PdfVersion version)
{
  const char *cstring = cairo_pdf_version_to_string(static_cast<cairo_pdf_version_t>(version));
  return cstring ? std::string(cstring) : std::string();
}

#endif // CAIRO_HAS_PDF_SURFACE




#ifdef CAIRO_HAS_PS_SURFACE

PsSurface::PsSurface(cairo_surface_t* cobject, bool has_reference) :
    Surface(cobject, has_reference)
{}

PsSurface::~PsSurface()
{
  // surface is destroyed in base class
}

RefPtr<PsSurface> PsSurface::create(std::string filename, double width_in_points, double height_in_points)
{
  auto cobject = cairo_ps_surface_create(filename.c_str(), width_in_points, height_in_points);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<PsSurface>(new PsSurface(cobject, true /* has reference */));
}

RefPtr<PsSurface> PsSurface::create_for_stream(const SlotWriteFunc& write_func, double
                                               width_in_points, double height_in_points)
{
  auto slot_copy = new SlotWriteFunc(write_func);
  auto cobject =
    cairo_ps_surface_create_for_stream(write_func_wrapper, slot_copy,
                                       width_in_points, height_in_points);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  set_write_slot(cobject, slot_copy);
  return make_refptr_for_instance<PsSurface>(new PsSurface(cobject, true /* has reference */));
}

void PsSurface::set_size(double width_in_points, double height_in_points)
{
  cairo_ps_surface_set_size(cobj(), width_in_points, height_in_points);
  check_object_status_and_throw_exception(*this);
}


void PsSurface::dsc_comment(std::string comment)
{
  cairo_ps_surface_dsc_comment(cobj(), comment.c_str());
  check_object_status_and_throw_exception(*this);
}

void PsSurface::dsc_begin_setup()
{
  cairo_ps_surface_dsc_begin_setup(cobj());
  check_object_status_and_throw_exception(*this);
}

void PsSurface::dsc_begin_page_setup()
{
  cairo_ps_surface_dsc_begin_page_setup(cobj());
  check_object_status_and_throw_exception(*this);
}

bool PsSurface::get_eps() const
{
  auto result = cairo_ps_surface_get_eps(const_cast<cairo_surface_t*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return result;
}

void PsSurface::set_eps(bool eps)
{
  cairo_ps_surface_set_eps(cobj(), eps);
  check_object_status_and_throw_exception(*this);
}

void PsSurface::restrict_to_level(PsLevel level)
{
  cairo_ps_surface_restrict_to_level(cobj(), static_cast<cairo_ps_level_t>(level));
  check_object_status_and_throw_exception(*this);
}

const std::vector<PsLevel> PsSurface::get_levels()
{
  cairo_ps_level_t const *levels;
  int num_levels;
  cairo_ps_get_levels(&levels, &num_levels);

  // Just copy the level array out into a std::vector.  This is a rarely used
  // function and the array of levels is going to be very small, so there's no
  // real performance hit.
  std::vector<PsLevel> vec;
  for (int i = 0; i < num_levels; ++i)
  {
    vec.push_back(static_cast<PsLevel>(levels[i]));
  }
  return vec;
}

std::string PsSurface::level_to_string(PsLevel level)
{
  return std::string(cairo_ps_level_to_string(static_cast<cairo_ps_level_t>(level)));
}

#endif // CAIRO_HAS_PS_SURFACE




#ifdef CAIRO_HAS_SVG_SURFACE

SvgSurface::SvgSurface(cairo_surface_t* cobject, bool has_reference) :
    Surface(cobject, has_reference)
{}

SvgSurface::~SvgSurface()
{
  // surface is destroyed in base class
}

RefPtr<SvgSurface> SvgSurface::create(std::string filename, double width_in_points, double height_in_points)
{
  auto cobject = cairo_svg_surface_create(filename.c_str(), width_in_points, height_in_points);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<SvgSurface>(new SvgSurface(cobject, true /* has reference */));
}

RefPtr<SvgSurface> SvgSurface::create_for_stream(const SlotWriteFunc& write_func,
                                                 double width_in_points,
                                                 double height_in_points)
{
  auto slot_copy = new SlotWriteFunc(write_func);
  auto cobject =
    cairo_svg_surface_create_for_stream(write_func_wrapper, slot_copy,
                                        width_in_points, height_in_points);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  set_write_slot(cobject, slot_copy);
  return make_refptr_for_instance<SvgSurface>(new SvgSurface(cobject, true /* has reference */));
}

void SvgSurface::restrict_to_version(SvgVersion version)
{
  cairo_svg_surface_restrict_to_version(cobj(), static_cast<cairo_svg_version_t>(version));
  check_object_status_and_throw_exception(*this);
}

const std::vector<SvgVersion> SvgSurface::get_versions()
{
  cairo_svg_version_t const *versions;
  int num_versions;
  cairo_svg_get_versions(&versions, &num_versions);

  // Just copy the version array out into a std::vector.  This is a rarely used
  // function and the array of versions is going to be very small, so there's no
  // real performance hit.
  std::vector<SvgVersion> vec;
  for (int i = 0; i < num_versions; ++i)
  {
    vec.push_back(static_cast<SvgVersion>(versions[i]));
  }
  return vec;
}

std::string SvgSurface::version_to_string(SvgVersion version)
{
  return std::string(cairo_svg_version_to_string(static_cast<cairo_svg_version_t>(version)));
}

#endif // CAIRO_HAS_SVG_SURFACE




#ifdef CAIRO_HAS_GLITZ_SURFACE

GlitzSurface::GlitzSurface(cairo_surface_t* cobject, bool has_reference)
: Surface(cobject, has_reference)
{ }

GlitzSurface::~GlitzSurface()
{
  // surface is destroyed in base class
}

RefPtr<GlitzSurface> GlitzSurface::create(glitz_surface_t *surface)
{
  auto cobject = cairo_glitz_surface_create(surface);
  check_status_and_throw_exception(cairo_surface_status(cobject));
  return make_refptr_for_instance<GlitzSurface>(new GlitzSurface(cobject, true /* has reference */));
}

#endif // CAIRO_HAS_GLITZ_SURFACE

} //namespace Cairo

// vim: ts=2 sw=2 et
