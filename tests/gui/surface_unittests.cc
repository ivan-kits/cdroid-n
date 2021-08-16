#include <gtest/gtest.h>
#include <cdtypes.h>
#include <cdgraph.h>
#include <core/canvas.h>
#include <sys/time.h>
#include <core/assets.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <core/layout.h>
#include <dirent.h>
#include <svg-cairo.h>
#include <fstream>
#include <core/textutils.h>
#include <fribidi.h>
using namespace cdroid;
#define SLEEP(x) usleep((x)*1000)
class CONTEXT:public testing::Test{
protected:
public :
   static Assets *rm;
   static Canvas*ctx;
   static void SetUpTestCase(){
       GFXInit();
       ctx=GraphDevice::getInstance().createContext(1280,720);
       rm=new Assets("ntvplus.pak");
   }
   static void TearDownCase(){
       delete rm; 
	   delete ctx;
   }
   virtual void SetUp(){
	   ctx->set_color(0xFF000000);
	   ctx->rectangle(0,0,800,600);
	   ctx->fill();
   }
   virtual void TearDown(){
		RECT rect={0,0,1280,720};
        //GraphDevice::getInstance().invalidate(&rect);
        GraphDevice::getInstance().ComposeSurfaces();
        sleep(2);
   }
};
Canvas*CONTEXT::ctx=nullptr;
Assets *CONTEXT::rm=nullptr;

TEST_F(CONTEXT,SURFACE_CREATE_1){
   const char *piaoyao="المعالجة الأولية Hello";
   for(int j=0;j<10;j++){
      for(int i=0;i<10;i++){
         const char *txt[]={"Beijing","Sigapo","ShangHai","Shenzhen",
            "The quick brown fox jumps over a lazy dog",
            "Innovation in China","中国智造，惠及全球 0123456789"};
         ctx->set_font_size(i==j?40:28);
         ctx->save();
         ctx->select_font_face("DejaVu Sans Mono",ToyFontFace::Slant::NORMAL,ToyFontFace::Weight::NORMAL);
         if(i==j){
             RECT rc={400,i*40,400,40};
             ctx->set_color(255,0,0);
             ctx->rectangle(0,i*40,800,40);
             ctx->fill();
             ctx->set_color(255,255,255);
             ctx->draw_text(10,i*40,txt[i%(sizeof(txt)/sizeof(char*))]);
         }else{
             ctx->rectangle(0,i*40,800,40);
             ctx->fill();
             ctx->set_color(255,255,255);
             ctx->draw_text(10,i*40,txt[i%(sizeof(txt)/sizeof(char*))]);
         }
      RECT rect={0,0,800,600};
      ctx->set_font_size(40);
      ctx->draw_text(100,480,piaoyao);//,DT_BOTTOM|DT_LEFT);
      //GraphDevice::getInstance().invalidate(&rect);
      ctx->restore();
      ctx->dump2png("test2.png");
      }
      GraphDevice::getInstance().ComposeSurfaces();
      SLEEP(500);
   }
   ctx->dump2png("test3.png");
}

TEST_F(CONTEXT,roundrect){
	ctx->set_color(0xFFFF0000);
	ctx->roundrect(50,50,500,400,20);
	ctx->stroke();
}

TEST_F(CONTEXT,TextOutXY){
    const char*str="jump over a dog";
    char s2[2],*p=(char*)str;
    RECT rect={100,100,800,120};
    ctx->set_font_size(80);
    ctx->set_color(0xFFFFFFFF);
    ctx->rectangle(100,100,800,1);
    ctx->fill();
    for(int x=100;*p;p++,x+=55){
       s2[0]=*p;s2[1]=0;
       ctx->draw_text(x,100,s2);
    }
    SLEEP(10000);
}

TEST_F(CONTEXT,drawtext){
    const char*str[]={"jump","over","a", "dog",nullptr};
    const char*str1="jump over a dog";
    int fmt[]={DT_TOP,DT_VCENTER,DT_BOTTOM};
    int fmth[]={DT_LEFT,DT_CENTER,DT_RIGHT};
    char s2[2],*p=(char*)str;
    RECT rect={100,100,800,120};
    ctx->set_font_size(80);
    for(int i=0;i<3;i++){
      rect.x=100;
      rect.y=100+150*i;
      ctx->set_color(0xFFFFFFFF);
      ctx->rectangle(rect);
      ctx->stroke();
      for(int j=0;j<3;j++){ 
         rect.x=100;
         ctx->set_color(0xFF000000|(255<<j*8));
         if(i)printf("%d,%d\r\n",rect.x,rect.y);
         if(i)ctx->draw_text(rect,str1,fmt[j]|fmth[i]);
         for(int k=0;i==0&&str[k];k++){
            ctx->draw_text(rect,str[k],fmt[j]|fmth[i]);
            rect.x+=strlen(str[k])*52;
         }
      }
    }
}


TEST_F(CONTEXT,memleak1){
    RECT rect={0,0,1000,80};
    SLEEP(10000);
    for(int i=0;i<1000000;i++){
       ctx->draw_text(rect,"The quick brown fox jumps over a lazy dog",DT_CENTER|DT_VCENTER);
    }
}

TEST_F(CONTEXT,TEXT_ALIGNMENT){
    const char*horz[]={"LEFT","CENTER","RIGHT"};
    const char*vert[]={"TOP","VCENTER","BOTTOM"};
    char alignment[64];
    RECT rect={100,100,800,120};
    ctx->set_font_size(40);
    for(int h=0;h<=2;h++){
        for(int v=0;v<=2;v++){
           ctx->set_color(0xFFFFFFFF);
           ctx->rectangle(0,0,1080,720);
           ctx->fill();
           sprintf(alignment,"%s_%s",horz[h],vert[v]);
           printf("test %s\r\n",alignment);
           ctx->set_color(0xFFFF0000);
           ctx->draw_text(20,20,alignment);
           ctx->rectangle(rect);ctx->fill();
           ctx->set_color(0xFF00FF00);
           ctx->draw_text(rect,"The quick brown fox jump sover the lazy dog.",(v<<4)|h);
           //GraphDevice::getInstance().invalidate(&rect);
           strcat(alignment,".png");
           ctx->dump2png(alignment);
           GraphDevice::getInstance().ComposeSurfaces();
           SLEEP(1000);
        }SLEEP(1000); 
    }
    SLEEP(2000);
}


TEST_F(CONTEXT,subcanvas){
    RefPtr<Canvas>ctx1(ctx->subContext(100,100,300,200));
    RefPtr<Canvas>ctx2(ctx1->subContext(100,100,100,100));
    RefPtr<Canvas>ctx3(ctx2->subContext(50,50,50,50));
    ctx->set_color(0xFFFFFFFF);
    ctx->rectangle(0,0,800,600);ctx->fill();
    ctx1->set_color(0xFF00FF00);
    ctx1->rectangle(0,0,300,200);ctx1->fill();
    ctx2->set_color(0xFF0000FF);
    ctx2->rectangle(0,0,100,100);ctx2->fill();
    ctx3->set_color(0xFFFF0000);
    ctx3->rectangle(0,0,50,50);ctx3->fill();
}
TEST_F(CONTEXT,circle){
    const int PTCOUNT=30;
    const double RADIUS=280;
    struct CPoint{
        double x;
        double y;
    };
    CPoint pts[PTCOUNT];
    for(int i=0;i<PTCOUNT;i++){
        pts[i].x=RADIUS*cos(M_PI*2.f*i/PTCOUNT);
        pts[i].y=RADIUS*sin(M_PI*2.f*i/PTCOUNT);
    }
    ctx->set_source_rgb(.1,.1,.1);
    ctx->rectangle(0,0,800,600);
    ctx->fill();
    ctx->translate(300,300);
    ctx->set_source_rgb(0,1,0);
    ctx->arc(0,0,RADIUS,0,M_PI*2.);
    ctx->stroke();
    for(int i=0;i<PTCOUNT;i++){
        ctx->set_source_rgba((float)i/PTCOUNT,(float)(PTCOUNT-i)/PTCOUNT,0,0.4);
        for(int j=i+1;j<PTCOUNT;j++){
            ctx->move_to(pts[i].x,pts[i].y);
            ctx->line_to(pts[j].x,pts[j].y);
        }
        ctx->stroke();
    }
    ctx->translate(-300,-300);
    RECT r={0,0,800,600};
    //GraphDevice::getInstance().invalidate(&r);
    GraphDevice::getInstance().ComposeSurfaces();
}

TEST_F(CONTEXT,Translate){
    RECT rect={0,0,800,600};
    ctx->set_color(0xFFFFFFFF);
    ctx->rectangle(0,0,800,600);ctx->fill();
    ctx->set_color(0xFFFF0000);
    ctx->rectangle(0,0,200,200);ctx->fill();

    ctx->translate(100,100);
    ctx->set_color(0xFF00FF00);
    ctx->rectangle(0,0,100,100);ctx->fill();

    ctx->translate(-50,-50);
    ctx->set_color(0xFF0000FF);
    ctx->rectangle(0,0,100,100);ctx->fill();
    
    ctx->translate(-50,-50);
    ctx->set_color(0xFFFFFF00);
    ctx->rectangle(0,0,100,100);
    ctx->stroke();
}

TEST_F(CONTEXT,Clip){
    RefPtr<ImageSurface>img=ImageSurface::create_from_png("light.png");
    RECT rect={0,0,800,600};
    ctx->set_color(0xFFFFFFFF);
    ctx->rectangle(0,0,800,600);ctx->fill();
    ctx->arc(400,300,100,0,M_PI*2);
    ctx->clip();
    ctx->draw_image(img,rect,nullptr);
}

TEST_F(CONTEXT,Mask){
    RefPtr<ImageSurface>img=ImageSurface::create_from_png("im_game.png");
    RefPtr<Pattern>pat1=SurfacePattern::create(img);
    RefPtr<Pattern>pat2=SolidPattern::create_rgba(0,1.0,0,.5);
    RECT rect={0,0,800,600};
    ctx->set_source(pat1);
    ctx->rectangle(0,0,800,600);
    ctx->mask(pat2);
    ctx->paint();//_with_alpha(0.5);
}

TEST_F(CONTEXT,ImageSurface){
    RefPtr<ImageSurface>img=ImageSurface::create_from_png("im_game.png");
    RefPtr<Cairo::Context>ictx=Cairo::Context::create(img);
    RefPtr<Gradient>pat=LinearGradient::create(0,0,400,0);
    RefPtr<Pattern>pat1=SolidPattern::create_rgba(1,0,0,.51);
    pat->add_color_stop_rgba ( .0, 1, 1, 1, 0);
    pat->add_color_stop_rgba ( .2, 0, 1, 0, 0.5);
    pat->add_color_stop_rgba ( .4, 1, 1, 1, 0);
    pat->add_color_stop_rgba ( .6, 0, 0, 1, 0.5);
    pat->add_color_stop_rgba ( .8, 1, 1, 1, 0);
    ictx->set_source(pat1);
    ictx->rectangle(0,0,400,400);
    ictx->fill();
    ctx->set_source(img,0,0);
    ctx->rectangle(0,0,400,400);
    ctx->fill();
}
TEST_F(CONTEXT,Pattern_Line){
   int i, j; 
   RefPtr<RadialGradient>radpat(RadialGradient::create(200, 150, 80, 400, 300, 400));
   RefPtr<LinearGradient>linpat(LinearGradient::create(200, 210, 600, 390));
   RECT rect={0,0,800,600};
   radpat->add_color_stop_rgb ( 0, 1.0, 0.8, 0.8); 
   radpat->add_color_stop_rgb ( 1, 0.9, 0.0, 0.0); 
   for (i=1; i<10; i++) 
        for (j=1; j<10; j++) 
            ctx->rectangle ( i*50, j*50,48 ,48); 
   ctx->set_source ( radpat); 
   ctx->fill (); 
   linpat->add_color_stop_rgba ( .0, 1, 1, 1, 0); 
   linpat->add_color_stop_rgba ( .2, 0, 1, 0, 0.5); 
   linpat->add_color_stop_rgba ( .4, 1, 1, 1, 0); 
   linpat->add_color_stop_rgba ( .6, 0, 0, 1, 0.5); 
   linpat->add_color_stop_rgba ( .8, 1, 1, 1, 0); 
   ctx->rectangle ( 0, 0, 800, 600); 
   ctx->set_source(linpat); 
   ctx->fill ();
   ctx->dump2png("pat-line.png");
}
TEST_F(CONTEXT,Pattern_Radio){
   RefPtr<RadialGradient>radpat(RadialGradient::create(200, 200, 10, 200, 200, 150));
   RECT rect={0,0,1200,600};
   radpat->add_color_stop_rgb ( .0, 1., 1., 1.);
   radpat->add_color_stop_rgb ( 1., 1., .0,.0);
   ctx->set_source ( radpat);
   ctx->arc(200,200,150,.0,3.1415*2);
   ctx->fill ();
   ctx->dump2png("pat-radio.png");
}

TEST_F(CONTEXT,Font){
    const char *texts[]={
        "The quick brown fox jumps over a lazy dog",
        "Innovation in China 0123456789" 
    };
    ctx->set_color(0xffffffff);
    ctx->rectangle(0,0,1280,720);ctx->fill();
    RECT rect={0,0,1280,720};
    auto lg=LinearGradient::create(10,480,1100,720);
    lg->add_color_stop_rgba(.0,1.0,0,0,0.5);
    lg->add_color_stop_rgba(.5,.0,1.,.0,1.);
    lg->add_color_stop_rgba(1.,.0,0,.1,0.5);
    for(int i=0,y=10;i<10;i++){
       std::string txt=texts[i%(sizeof(texts)/sizeof(char*))];
       int font_size=10+i*5;
       //cant use this select_font_face use defaut font pls.
       //ctx->select_font_face(faces[i],FONT_SLANT_NORMAL, FONT_WEIGHT_BOLD);
       ctx->set_font_size(font_size);
       ctx->move_to(0,y);y+=font_size+10;
       if(i%4==0)ctx->set_color(0,0,0);
       else ctx->set_source(lg);
       if(i%2==0){
           ctx->show_text(txt);
       }else{
           ctx->text_path(txt);ctx->stroke();
       }
       GraphDevice::getInstance().ComposeSurfaces();
        SLEEP(200);
    }
    
    ctx->set_font_size(150);
    for(int i=0;i<20;i++){
       auto lg=LinearGradient::create(20*i,480,300+50*i,720);
       lg->add_color_stop_rgba(.0,1.0,0,0,0.5);
       lg->add_color_stop_rgba(.5,.0,1.,.0,1.);
       lg->add_color_stop_rgba(1.,.0,0,.1,0.5);
       ctx->set_source(lg);
       ctx->move_to(0,600);
       ctx->show_text("Innovation in China!");
       //GraphDevice::getInstance().invalidate(&rect);
       GraphDevice::getInstance().ComposeSurfaces();
       SLEEP(100);
    }
    SLEEP(1000);
}
TEST_F(CONTEXT,ALPHA){
     ctx->set_color(0xFFFFFFFF);
     ctx->rectangle(0,0,1280,720);ctx->fill();
     ctx->set_color(0x80FF0000);
     ctx->rectangle(200,200,480,320);ctx->fill();
     ctx->dump2png("alpha.png");
}
TEST_F(CONTEXT,HOLE){
     ctx->set_color(0xFFFFFFFF);
     ctx->rectangle(0,0,1280,720);ctx->fill();
     ctx->set_color(0);
     ctx->rectangle(200,200,480,320);ctx->fill();
     ctx->dump2png("hole.png");
}
TEST_F(CONTEXT,HOLE2){
     ctx->set_color(0xFFFFFFFF);
     ctx->rectangle(0,0,1280,720);ctx->fill();
     ctx->set_source_rgba(0,0,0,0);
     ctx->rectangle(200,200,480,320);ctx->fill();
     ctx->dump2png("hole2.png");
}
TEST_F(CONTEXT,HOLE3){
     RefPtr<ImageSurface>img=ImageSurface::create(Surface::Format::ARGB32,1280,720);
     RefPtr<Cairo::Context>ctx1=Cairo::Context::create(img);
     ctx1->set_source_rgb(1,0.5,1);
     ctx1->rectangle(0,0,1280,720);
     ctx1->fill();
     ctx1->set_source_rgba(0,1,0,0.1);
     ctx1->rectangle(200,200,480,320);
     ctx1->fill();
     img->write_to_png("hole3.png");
}
TEST_F(CONTEXT,layout){
    Layout layout(30,200);
    ctx->set_font_size(30);
    layout.setText("A brown fox jump over the lazy dog!");
    ctx->set_source_rgb(0,0,0);
    ctx->rectangle(0,0,1280,720);
    ctx->fill();
    ctx->set_source_rgb(1,0,0);
    layout.draw(*ctx);
    for(int i=0;i<20;i++){
        int off=layout.getOffsetToLeftOf(i);//getLineForOffset(i);
        printf("[%02d] offset=%d\r\n",i,off);
    }
}

#if 0
void DrawBitmap(RefPtr<ImageSurface>img,FT_Bitmap*bmp,int x,int y){
    uint8_t*pixels=(img->get_data()+img->get_stride()*y+x*4);
    for(int y=0;y<bmp->rows;y++){
        unsigned char*pc=bmp->buffer+y*bmp->pitch;
        for(int x=0;x<bmp->width;x++)
           if(pc[x]){pixels[x*4]=0xFF;pixels[x*4+1]=00;pixels[x*4+2]=0;pixels[x*4+3]=pc[x];}
           else {pixels[x*4]=pixels[x*4+1]=pixels[x*4+2]=pixels[x*4+3]=0;}
        pixels+=img->get_stride();
    }
}
static const std::wstring processBidi(const std::wstring&logstr){
#ifdef ENABLE_FRIBIDI
    size_t wsize=logstr.length()+1;
    FriBidiCharType base_dir=FRIBIDI_TYPE_ON;
    FriBidiChar * visstr= new FriBidiChar[wsize] ;
    FriBidiLevel* level = new FriBidiLevel[wsize];
    FriBidiStrIndex *posLV= new FriBidiStrIndex[wsize];
    FriBidiStrIndex *posVL= new FriBidiStrIndex[wsize];

    fribidi_log2vis((const FriBidiChar*)logstr.c_str(),logstr.length(),&base_dir,visstr,posLV,posVL,level);
    std::wstring biditxt((const wchar_t*)visstr);
    delete [] visstr;
    delete [] posLV;
    delete [] posVL;
    delete [] level;
    return biditxt;
#else
    return TextUtils::unicode2utf8(logstr);
#endif
}
#include <hb.h>
#include <hb-ft.h>
TEST_F(CONTEXT,hebrew){
#define DOUBLE_FROM_26_6(t) ((double)(t) / 64.0)
#define HBFixedToFloat(t)  ((double)(t)/64.0)
    const wchar_t text[]={0xfb49,0x05d0,0x05ad,0x0020,0x05d0,0x059d,0x0020,0x05d0,0x0599,0x0020,
             0x0020,0x05d0,0x05b6,0x0020,0xfb30,0x0020,0x05d0,0x0592,0x0020,0x05dc,
             0x05ad,0x0020,0x05dc,0x059d,0x0020,0x05dc,0x0599,0x0020,0x05dc,0x05b6,
             0x0020,0xfb3c,0x0020,0x05dc,0x0592,0x00};
    RefPtr<ImageSurface>imgsurface=ImageSurface::create(Surface::Format::ARGB32,800,600);
    RefPtr<Cairo::Context>ctx=Cairo::Context::create(imgsurface);
    std::string u8text=TextUtils::unicode2utf8(text);
    TextExtents extents;
    const wchar_t *p=text;
    FT_Vector kerning;

    FT_Library library;
    FT_Init_FreeType(&library);
    FT_Face ftface;
    FT_Error fterror;
    fterror=FT_New_Face(library,"/home/houzh/Miniwin1/env/fonts/droidfallback.ttf",0,&ftface);

    printf("fterror=%d ftface=%p\r\n",fterror,ftface);
    hb_face_t* hbface=hb_ft_face_create(ftface,nullptr);
    hb_font_t *hbfont=hb_font_create (hbface);
    hb_buffer_t *buffer=hb_buffer_create();
    hb_glyph_info_t *glyphinfos;
    hb_glyph_position_t *positions;

    hb_buffer_set_direction (buffer, HB_DIRECTION_LTR);
    size_t len=sizeof(text)/sizeof(wchar_t);
    std::wstring wtext=processBidi(text);
    hb_buffer_add_utf32(buffer,(const uint32_t*)wtext.c_str(),len,0,len);
    hb_font_set_ppem(hbfont,40,40);
    hb_shape (hbfont, buffer, NULL, 0);
    len = hb_buffer_get_length (buffer);
    glyphinfos = hb_buffer_get_glyph_infos (buffer, NULL);
    positions = hb_buffer_get_glyph_positions (buffer, NULL);

    ctx->set_font_size(40);
    ctx->set_source_rgb(0,1,0);


    FT_Set_Char_Size(ftface, 40*64, 40*64, 96, 96);
    float xx=20; 
    for(int i=0;i<len;i++){
        fterror=FT_Load_Glyph(ftface, glyphinfos[i].codepoint, FT_LOAD_DEFAULT|FT_LOAD_TARGET_LCD);
        if (ftface->glyph->format != FT_GLYPH_FORMAT_BITMAP)
	    fterror=FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_NORMAL);
        xx+=DOUBLE_FROM_26_6(ftface->glyph->bitmap_left)+DOUBLE_FROM_26_6(ftface->glyph->advance.x);
        printf("%d left=%f,%f,%f yadvance=%f\r\n",i,DOUBLE_FROM_26_6(ftface->glyph->bitmap_left),DOUBLE_FROM_26_6(ftface->glyph->advance.x),
            DOUBLE_FROM_26_6(ftface->glyph->bitmap_top),HBFixedToFloat(positions[i].y_advance));
        FT_Bitmap*bmp=&ftface->glyph->bitmap;
        DrawBitmap(imgsurface,bmp,xx,100+HBFixedToFloat(positions[i].y_offset)+HBFixedToFloat(positions[i].y_advance)
          +/*DOUBLE_FROM_26_6(ftface->size->metrics.ascender)+*/DOUBLE_FROM_26_6(ftface->glyph->bitmap_top));
    }
    cairo_stroke(ctx->cobj());
    imgsurface->write_to_png("hebrew.png"); 
}
#endif
