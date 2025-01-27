#include <color.h>
#include <drawables.h>
#include <expat.h>
#include <fstream>
#include <cdlog.h>
#include <string.h>
#include <textutils.h>

using namespace Cairo;
namespace cdroid {

Drawable::ConstantState::~ConstantState() {
}

Drawable::Drawable() {
    mLevel = 0;
    mChangingConfigurations = 0;
    mVisible = true;
    mLayoutDirection = LayoutDirection::LTR;
    mCallback = nullptr;
    mBounds.set(0,0,0,0);
    mColorFilter = nullptr;
}

Drawable::~Drawable() {
    delete mColorFilter;
}

void Drawable::setBounds(const Rect&r) {
    setBounds(r.left,r.top,r.width,r.height);
}

void Drawable::setBounds(int x,int y,int w,int h) {
    if((mBounds.left!=x)||(mBounds.top!=y)||(mBounds.width!=w)||(mBounds.height!=h)) {
        if(!mBounds.empty())
            invalidateSelf();
        mBounds.set(x,y,w,h);
        onBoundsChange(mBounds);
    }
}

bool Drawable::getPadding(Rect&padding) {
    padding.set(0,0,0,0);
    return false;
}

Insets Drawable::getOpticalInsets() {
    return Insets();
}

const Rect&Drawable::getBounds()const {
    return mBounds;
}

Rect Drawable::getDirtyBounds() {
    return mBounds;
}

Drawable*Drawable::mutate() {
    return this;
}

void Drawable::clearMutated() {
}

int Drawable::getOpacity() {
    return UNKNOWN;
}

void Drawable::setHotspot(float x,float y) {
}

void Drawable::setHotspotBounds(int left,int top,int width,int height) {
}

void Drawable::getHotspotBounds(Rect&outRect) {
    outRect=mBounds;
}

std::shared_ptr<Drawable::ConstantState>Drawable::getConstantState() {
    return nullptr;
}

void Drawable::setAutoMirrored(bool mirrored) {
}

bool Drawable::isAutoMirrored() {
    return false;
}

void Drawable::setColorFilter(ColorFilter*cf) {
    delete mColorFilter;
    mColorFilter = cf;
    invalidateSelf();
    LOGV("setColorFilter %p:%p",this,cf);
}

ColorFilter*Drawable::getColorFilter(){
    return nullptr;
}

void Drawable::clearColorFilter(){
    setColorFilter(nullptr);
}

void Drawable::setColorFilter(int color,PorterDuffMode mode) {
    setColorFilter(new PorterDuffColorFilter(color,mode));
}

void Drawable::setTint(int color) {
    ColorStateList*tint =ColorStateList::valueOf(color);
    setTintList(tint);
    delete tint;
}

PorterDuffColorFilter *Drawable::updateTintFilter(PorterDuffColorFilter* tintFilter,ColorStateList* tint,int tintMode) {
    if ( (tint == nullptr) || (tintMode == TintMode::NOOP) ) {
        return nullptr;
    }

    const int color = tint->getColorForState(getState(), Color::TRANSPARENT);
    if (tintFilter == nullptr) {
        return new PorterDuffColorFilter(color, tintMode);
    }

    tintFilter->setColor(color);
    tintFilter->setMode(tintMode);
    return tintFilter;
}

void Drawable::setTintList(ColorStateList* tint) {
}

void Drawable::setTintMode(int mode) {
}

bool Drawable::isStateful()const {
    return false;
}

bool Drawable::hasFocusStateSpecified()const {
    return false;
}

bool Drawable::setState(const std::vector<int>&states) {
    mStateSet=states;
    return onStateChange(states);
}

const std::vector<int>& Drawable::getState()const {
    return mStateSet;
}

bool Drawable::setLevel(int level) {
    if(mLevel!=level) {
        mLevel=level;
        return onLevelChange(level);
    }
    return false;
}

int Drawable::getMinimumWidth()const {
    const int intrinsicWidth = getIntrinsicWidth();
    return intrinsicWidth > 0 ? intrinsicWidth : 0;
}

int Drawable::getMinimumHeight()const {
    const int intrinsicHeight = getIntrinsicHeight();
    return intrinsicHeight > 0 ? intrinsicHeight : 0;
}

bool Drawable::setLayoutDirection (int dir) {
    if (mLayoutDirection != dir) {
        mLayoutDirection = dir;
        return onLayoutDirectionChanged(dir);
    }
    return false;
}

int Drawable::getLayoutDirection()const {
    return mLayoutDirection;
}

int Drawable::getIntrinsicWidth()const {
    return -1;
}

int Drawable::getIntrinsicHeight()const {
    return -1;
}

bool Drawable::isVisible()const {
    return mVisible;
}

bool Drawable::setVisible(bool visible, bool restart) {
    const bool changed = mVisible != visible;
    if (changed) {
        mVisible = visible;
        invalidateSelf();
    }
    return changed;
}

int Drawable::getChangingConfigurations()const {
    return mChangingConfigurations;
}

void Drawable::setChangingConfigurations(int configs) {
    mChangingConfigurations =configs;
}

void Drawable::setCallback(Drawable::Callback*cbk) {
    mCallback=cbk;
}

Drawable::Callback* Drawable::getCallback()const {
    return mCallback;
}

void Drawable::scheduleSelf(Runnable& what, long when) {
    if(mCallback)mCallback->scheduleDrawable(*this, what, when);
}

void Drawable::unscheduleSelf(Runnable& what) {
    if(mCallback)mCallback->unscheduleDrawable(*this, what);
}

void Drawable::invalidateSelf() {
    if(mCallback)mCallback->invalidateDrawable(*this);
}

void Drawable::jumpToCurrentState() {
}

Drawable*Drawable::getCurrent() {
    return this;
}

float Drawable::scaleFromDensity(float pixels, int sourceDensity, int targetDensity) {
    return pixels * targetDensity / sourceDensity;
}

int Drawable::scaleFromDensity(int pixels, int sourceDensity, int targetDensity, bool isSize) {
    if (pixels == 0 || sourceDensity == targetDensity)
        return pixels;

    const float result = pixels * targetDensity / (float) sourceDensity;
    if (!isSize)  return (int) result;

    const int rounded = round(result);
    if (rounded != 0)     return rounded;
    else if (pixels > 0) return 1;
    else return -1;
}

////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////// Drawable inflate//////////////////////////////////////////////

typedef std::function<Drawable*(Context*ctx, const AttributeSet&attrs)>DrawableParser;

class ParseItem {
  public:
    Drawable*drawable;
    AttributeSet props;
    std::string name;
    ParseItem() {
        drawable=nullptr;
    }
};

class ParseData {
  public:
    std::vector<std::shared_ptr<ParseItem> >items;
    std::vector<AttributeSet> attrs;
    std::vector<std::string>  names;
    Drawable* drawable;
    XML_Parser parser;
    Context* ctx;
    std::string package;
    std::string resourceFile;
    ParseData() {
        drawable = nullptr;
        parser = nullptr;
    }
    void push(const std::string&name,AttributeSet&atts) {
        std::shared_ptr<ParseItem> item = std::make_shared<ParseItem>();
        item->name  = name;
        item->props = atts;
        items.push_back(item);
    }
    bool upperIsItem()const {
        return (items.size()>1) && (items.at(items.size()-2)->name.compare("item")==0);
    }
    bool upperIsShape()const {
        return (items.size()>1) && (items.at(items.size()-2)->name.compare("shape")==0);
    }
    bool upperIsWrapper()const {
        if(items.size()<=1)return false;
        auto upper = items.at(items.size()-2);
        return upper->drawable && dynamic_cast<DrawableWrapper*>(upper->drawable);
    }
    bool _upperIsContainer() {
        if(items.size()<=1)return false;
        auto upper = items.at(items.size()-2);
    }
    void pop2Upper(Drawable*d) {
        auto upper=items.at(items.size()-2);
        if(upperIsItem()) upper->drawable=d;
        else if(upperIsWrapper()) {
            DrawableWrapper* dwrap = dynamic_cast<DrawableWrapper*>(upper->drawable);
            dwrap->setDrawable(d);
        }
    }
};


Drawable*Drawable::createWrappedDrawable(Context*ctx,const AttributeSet&atts) {
    RefPtr<ImageSurface> img;
    std::string resname = atts.getString("color");
    if(!resname.empty()) {
        int color=Color::parseColor(resname);
        return new ColorDrawable(color);
    }
    resname=atts.getString("drawable");
    if(resname.empty())
        return nullptr;

    if(resname.find("xml")!=std::string::npos) {
        return Drawable::inflate(ctx,resname);
    }
    Drawable*dr=ctx->getDrawable(resname);
    dr->mResourceId=resname;
    return dr;
}

static std::map<const std::string,DrawableParser>drawableParsers= {
    {"color", ColorDrawable::inflate},
    {"shape", ShapeDrawable::inflate},
    {"bitmap", BitmapDrawable::inflate},
    {"nine-patch", NinePatchDrawable::inflate},
    {"inset", InsetDrawable::inflate},
    {"scale", ScaleDrawable::inflate},
    {"rotate", RotateDrawable::inflate},
    {"clip", ClipDrawable::inflate},
    {"transition", TransitionDrawable::inflate},
    {"layer-list", LayerDrawable::inflate},
    {"level-list", LevelListDrawable::inflate},
    {"selector", StateListDrawable::inflate},
    {"item", Drawable::createWrappedDrawable },
    {"ripple", RippleDrawable::inflate},
    {"animated-rotate", AnimatedRotateDrawable::inflate},
    {"animation-list", AnimationDrawable::inflate}
};

static int parseColor(const std::string&value) {
    int color=Color::parseColor(value);
    return color;
}

static void parseShapeGradient(GradientDrawable*gd,ShapeDrawable*sd,const AttributeSet&atts) {
    std::vector<int32_t> cls;
    PointF center;
    GradientDrawable::Orientation orientation= GradientDrawable::TOP_BOTTOM/*DEFAULT_ORIENTATION*/;
    cls.push_back(atts.getColor("startColor",0));
    if(atts.hasAttribute("centerColor"))
        cls.push_back(atts.getColor("centerColor"));
    if(atts.hasAttribute("endColor"))
        cls.push_back(atts.getColor("endColor",0));

    const int gradientType=atts.getInt("type",std::map<const std::string,int> {
        {"linear",GradientDrawable::LINEAR_GRADIENT},
        {"radial",GradientDrawable::RADIAL_GRADIENT},
        {"sweep",GradientDrawable::SWEEP_GRADIENT}
    },GradientDrawable::LINEAR_GRADIENT);

    const int angle = (atts.getInt("angle",0)%360+360)%360;

    switch(angle) {
    case 0 :
        orientation = GradientDrawable::LEFT_RIGHT;
        break;
    case 45:
        orientation = GradientDrawable::BL_TR     ;
        break;
    case 90:
        orientation = GradientDrawable::BOTTOM_TOP;
        break;
    case 135:
        orientation = GradientDrawable::BR_TL     ;
        break;
    case 180:
        orientation = GradientDrawable::RIGHT_LEFT;
        break;
    case 225:
        orientation = GradientDrawable::TR_BL     ;
        break;
    case 270:
        orientation = GradientDrawable::TOP_BOTTOM;
        break;
    case 315:
        orientation = GradientDrawable::TL_BR     ;
        break;
    }
    if(gd) {
        gd->setColors(cls);
        gd->setGradientType(gradientType);
        switch(gradientType) {
        case GradientDrawable::LINEAR_GRADIENT:
            gd->setOrientation(orientation);
            break;
        case GradientDrawable::RADIAL_GRADIENT:
            center.set( atts.getFloat("centerX"), atts.getFloat("centerY") );
            gd->setGradientCenter(center.x,center.y);
            gd->setGradientRadius(atts.getDimensionPixelSize("gradientRadius"));
            break;
        case GradientDrawable::SWEEP_GRADIENT:
            center.set( atts.getFloat("centerX"), atts.getFloat("centerY") );
            LOGD("center=(%f,%f)",center.x,center.y);
            gd->setGradientCenter(center.x,center.y);
            break;
        }
    } else if(sd) {
        //sd->setGradientColors(cls);
        //sd->setGradientCenterX(center.x);
        //sd->setGradientCenterY(center.y);
    }
}

static void parseCorners(GradientDrawable*gd,ShapeDrawable*sd,const AttributeSet&atts) {
    int radius = atts.getDimensionPixelSize("radius",-1);
    float topLeftRadius = atts.getDimensionPixelSize("topLeftRadius",radius);
    float topRightRadius= atts.getDimensionPixelSize("topRightRadius",radius);
    float bottomRightRadius= atts.getDimensionPixelSize("bottomRightRadius",radius);
    float bottomLeftRadius = atts.getDimensionPixelSize("bottomLeftRadius",radius);
    std::vector<float> radii = {topLeftRadius,topRightRadius,bottomRightRadius,bottomLeftRadius};
    if(gd)
        gd->setCornerRadii(radii);
}

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts) {
    ParseData* pd = (ParseData*)userData;
    auto it = drawableParsers.find(name);
    AttributeSet atts(pd->ctx,pd->package);
    atts.set(satts);
    pd->push(name,atts);

    if(it!=drawableParsers.end()) {
        auto item = pd->items.back();
        if(strcmp(name,"shape")||((strcmp(name,"item")==0)&&atts.hasAttribute("drawable")))
            item->drawable=it->second(pd->ctx,atts);
        LOGV("created drawable %s:%p props:%d",name,item->drawable,item->props.size());
    }
}

static Drawable*parseShapeDrawable(const AttributeSet&atts,const std::vector<AttributeSet>&props,const std::vector<std::string>&names) {
    const AttributeSet* corners = nullptr,*gradient = nullptr,*padding = nullptr;
    const AttributeSet* size = nullptr, *stroke = nullptr,*solid = nullptr;

    for(auto  i = 0 ; i < props.size() ; i++) {
        const AttributeSet& p = props.at(i);
        std::string tag = names.at(i);
        if(tag.compare("corners") ==0) corners = &p;
        if(tag.compare("gradient")==0) gradient= &p;
        if(tag.compare("size") ==0)   size = &p;
        if(tag.compare("stroke") ==0) stroke= &p;
        if(tag.compare("solid") ==0)  solid= &p;
        if(tag.compare("padding")==0) padding= &p;
    }

    const int shapeType = atts.getInt("shape",std::map<const std::string,int> {
        {"rectangle",GradientDrawable::Shape::RECTANGLE},{"oval",GradientDrawable::Shape::OVAL},
        {"line",GradientDrawable::Shape::LINE},  	 {"ring",GradientDrawable::Shape::RING}
    },GradientDrawable::Shape::RECTANGLE);

    LOGE_IF(!(gradient||solid||stroke),"stroke solid gradient property error!");
    if(gradient||solid||stroke) {
        GradientDrawable*d = new GradientDrawable();
        d->setShape(shapeType);
        d->setUseLevel(atts.getBoolean("useLevel"));

        if(corners)parseCorners(d,nullptr, *corners);
        if(shapeType == GradientDrawable::Shape::RING) {
            d->setInnerRadius(atts.getDimensionPixelSize("innerRadius"));
            d->setInnerRadiusRatio(atts.getDimensionPixelSize("innerRadiusRatio"));
            d->setThickness(atts.getDimensionPixelSize("thickness"));
            d->setThicknessRatio(atts.getDimensionPixelSize("thicknessRatio"));
        }

        if(gradient)parseShapeGradient(d,nullptr, *gradient);
        else d->setColor(solid?solid->getColor("color",0):0);

        if(size)d->setSize(size->getDimensionPixelSize("width",-1),size->getDimensionPixelSize("height",-1));

        if(stroke) {
            d->setStroke(stroke->getDimensionPixelSize("width",1),stroke->getColor("color",0),
                         stroke->getDimensionPixelSize("dashWidth"),stroke->getDimensionPixelSize("dashGap"));
        }
        if(padding)d->setPadding(padding->getInt("left"),padding->getInt("top"),
                                     padding->getInt("right"),padding->getInt("bottom"));
        return d;
    } else {
        ShapeDrawable*sd=new ShapeDrawable();
        if(corners) parseCorners(nullptr,sd, *corners);
        if(gradient)parseShapeGradient(nullptr,sd,*gradient);
        return sd;
    }
}

static void endElement(void *userData, const XML_Char *name) {
    ParseData*pd = (ParseData*)userData;

    auto it = drawableParsers.find(name);
    if(it == drawableParsers.end()) {
        auto pitem   = pd->items.back();
        pd->attrs.push_back(pitem->props);
        pd->names.push_back(pitem->name);
    } else if(pd->attrs.size()) { /*here coming shape drawable*/
        Drawable*leaf = parseShapeDrawable(pd->items.back()->props,pd->attrs,pd->names);
        pd->attrs.clear();
        pd->names.clear();
        if(pd->upperIsItem()||pd->upperIsWrapper()) {
            pd->pop2Upper(leaf);
        }
        pd->drawable= leaf;
        LOGV("coming drawable %s %p upperIsShape=%d",name,leaf,pd->upperIsShape());
    } else { /*item (stub drawable) or other drawable*/
        LOGV("coming drawable %s",name);
        Drawable* topchild = nullptr,*parent = nullptr;
        const AttributeSet atts = pd->items.back()->props;
        if(pd->upperIsItem()||pd->upperIsWrapper()) {
            pd->pop2Upper(pd->items.back()->drawable);
        }
        if(pd->items.size()>1) {
            auto pitem = pd->items.back();
            topchild = pitem->drawable;
            pitem  = pd->items[pd->items.size()-2];
            parent = pitem->drawable;
        }

        if(dynamic_cast<StateListDrawable*>(parent)) {
            std::vector<int> state;
            StateSet::parseState(state,atts);
            ((StateListDrawable*)parent)->addState(state,topchild);
            LOGV("add drawable %p to StateListDrawable %p",topchild,parent);
        } else if(dynamic_cast<LevelListDrawable*>(parent)) {
            int minLevel = atts.getInt("minLevel",INT_MIN);//get child level info
            int maxLevel = atts.getInt("maxLevel",INT_MIN);
            if( minLevel == INT_MIN ) minLevel = maxLevel;
            if( maxLevel == INT_MIN ) maxLevel = minLevel;
            ((LevelListDrawable*)parent)->addLevel(minLevel,maxLevel,topchild);
            LOGV("add drawable %p to LevelListDrawable %p level=(%d,%d)",topchild,parent,minLevel,maxLevel);
        } else if(dynamic_cast<LayerDrawable*>(parent)) {
            LayerDrawable* ld = dynamic_cast<LayerDrawable*>(parent);
            const int idx = ld->addLayer(topchild);
            ld->setLayerInset(idx,atts.getDimensionPixelOffset("left"),atts.getDimensionPixelOffset("top"),
                              atts.getDimensionPixelOffset("right"),atts.getDimensionPixelOffset("bottom"));
            ld->setLayerGravity(idx,atts.getGravity("gravity",Gravity::NO_GRAVITY));
            ld->setLayerWidth(idx,atts.getDimensionPixelOffset("width",-1));
            ld->setLayerHeight(idx,atts.getDimensionPixelOffset("height",-1));
            const int id = atts.getResourceId("id",-1);
            const std::string src = atts.getString("drawable");
            if(id!=-1)ld->setId(idx,id);
            LOGV("add drawable %p to LayerDrawable %p index=%d id=%d gravity=%x size=%dx%d",topchild,
                 parent,idx,id,ld->getLayerGravity(idx),ld->getLayerWidth(idx),ld->getLayerHeight(idx));
        } else if(dynamic_cast<AnimationDrawable*>(parent)) {
            AnimationDrawable* ad = (AnimationDrawable*)parent;
            const int duration = atts.getInt("duration",0);
            const std::string src = atts.getString("drawable");
            ad->addFrame(topchild,duration);
            LOGV("add drawable %p to AnimationDrawable %p duration=%d",topchild,parent,duration);
        }
        if(pd->items.size()==1)  pd->drawable=pd->items.back()->drawable;
    }
    pd->items.pop_back();//popup item
}

Drawable*Drawable::fromStream(Context*ctx,std::istream&stream,const std::string& resname,const std::string&package) {
    ParseData pd;
    int rdlen;
    char buf[256];
    XML_Parser parser = XML_ParserCreateNS(nullptr,' ');

    std::string basePath=resname.substr(0,resname.find_last_of("/"));
    basePath=basePath.substr(0,basePath.find_last_of("/"));

    pd.parser = parser;
    pd.ctx = ctx;
    pd.package = package;
    pd.resourceFile = resname;
    XML_SetUserData(parser,&pd);
    pd.items.clear();
    XML_SetElementHandler(parser, startElement, endElement);
    LOGE_IF(!stream.good(),"%s open failed",resname.c_str());
    do {
        stream.read(buf,sizeof(buf));
        rdlen=stream.gcount();
        if (XML_Parse(parser, buf,rdlen,!rdlen) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s at %s:line %ld",es, resname.c_str(),XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return nullptr;
        }
    } while(rdlen);
    XML_ParserFree(parser);
    pd.drawable->mResourceId=resname;
    LOGV("parsed drawable [%p] from %s",pd.drawable,resname.c_str());
    return pd.drawable;
}

Drawable*Drawable::inflate(Context*ctx,const std::string& resname) {
    Drawable*d = nullptr;
    if(ctx == nullptr) {
        std::ifstream fs(resname);
        if(fs.good())d = fromStream(ctx,fs,resname,"");
    } else if(!resname.empty()) {
        /*std::string package;
        std::unique_ptr<std::istream> is = ctx->getInputStream(resname,&package);
        d = fromStream(ctx,*is,resname,package);*/
        d = ctx->getDrawable(resname);
        LOGE_IF(d==nullptr,"%s load failed",resname.c_str());
    }
    return d;
}

}
