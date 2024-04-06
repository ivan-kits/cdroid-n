#include <assets.h>
#include <algorithm>
#include <cdtypes.h>
#include <cdlog.h>
#include <ziparchive.h>
#include <iostreams.h>
#include <iostream>
#include <fstream>
#include <drawables.h>
#include <textutils.h>
#include <expat.h>
#include <limits.h>
using namespace Cairo;
namespace cdroid {

Assets::Assets() {
    mNextAutofillViewId=100000;
}

Assets::Assets(const std::string&path):Assets() {
    addResource(path);
}

Assets::~Assets() {
    for(auto cls:mStateColors){
        delete cls.second;
    }
    mStateColors.clear();

    for(auto it=mResources.begin(); it!=mResources.end(); it++) {
        delete it->second;
    }
    for(auto d:mDrawables) {
        LOGD_IF(d.second.use_count(),"%s reference=%d",d.first.c_str(),d.second.use_count());
    }
    mDrawables.clear();
    mIDS.clear();
    mResources.clear();
    mStrings.clear();
    mStyles.clear();
    std::cout<<" Assets destroied!"<<std::endl;
}

const DisplayMetrics& Assets::getDisplayMetrics()const{
    return mDisplayMetrics;
}

const std::string Assets::getPackageName()const {
    return mName;
}

const std::string Assets::getTheme()const {
    return mThemeName;
}

void Assets::setTheme(const std::string&theme) {
    auto it=mStyles.find(theme);
    if(it!=mStyles.end()) {
        std::string pkg;
        mThemeName= theme;
        mTheme=it->second;
        parseResource(theme,nullptr,&pkg);
        LOGD("set Theme to %s",theme.c_str());
    } else {
        LOGE("Theme %s not found,[cdroid.pak %s] must be copied to your work directory!",theme.c_str(),
             mName.empty()?"":(mName+".pak").c_str());
    }
}

static std::string convertXmlToCString(const std::string& xml) {
    size_t pos;
    std::string result = xml;
    while((pos=result.find("\\n"))!=std::string::npos)
	result.replace(pos,2,"\n");
    while((pos=result.find("\\'"))!=std::string::npos)
	result.replace(pos,2,"\'");
    while((pos=result.find("\\\""))!=std::string::npos)
        result.replace(pos,2,"\"");
    return result;
}

void Assets::parseItem(const std::string&package,const std::vector<std::string>&tags,std::vector<AttributeSet>atts,const std::string&value) {
    const std::string&tag0=tags[0];
    if(atts.size()==1) {
        if(tag0.compare("id")==0) {
            const std::string name=package+":id/"+atts.back().getString("name");
            mIDS[name]=TextUtils::strtol(value);
            LOGV("%s=%s",name.c_str(),value.c_str());
        } else if(tag0.compare("color")==0) {
            const std::string name=atts[0].getString("name");
            uint32_t color = Color::parseColor(value);
            LOGV("%s:color/%s:%s",package.c_str(),name.c_str(),value.c_str());
            mColors.insert(std::pair<const std::string,uint32_t>(package+":color/"+name,color));
        } else if(tag0.compare("string")==0) {
            const std::string name= atts[0].getString("name");
            const std::string key = package+":string/"+name;
            LOGV_IF(!value.empty(),"%s =%s",key.c_str(),value.c_str());
            mStrings[key] = convertXmlToCString(value);
        }
    } else  if(atts.size()==2) {
        if(tag0.compare("style")==0) {
            AttributeSet&attStyle=atts[0];
            const std::string styleName  =package+":style/"+attStyle.getString("name");
            const std::string styleParent=attStyle.getString("parent");
            auto it=mStyles.find(styleName);
            if(it==mStyles.end()) {
                it=mStyles.insert(it,{styleName,AttributeSet()});
                if(styleParent.length())it->second.add("parent",styleParent);
                LOGV("style:%s",styleName.c_str());
            }
            std::string keyname=atts[1].getString("name");
            size_t pos =keyname.find(':');
            if(pos!=std::string::npos)keyname=keyname.substr(pos+1);
            const std::string normalizedValue = AttributeSet::normalize(package,value);
            it->second.add(keyname,normalizedValue);
        } else if(tag0.compare("array")==0) {
            const std::string name=atts[0].getString("name");
            const std::string key=package+":array/"+name;
            auto it=mArraies.find(key);
            if(it==mArraies.end()) {
                it=mArraies.insert(it,{key,std::vector<std::string>()});
                LOGV("array:%s",key.c_str());
            }
            it->second.push_back(value);
        } else if(tags[0].compare("string-array")==0) {
            const std::string name=atts[0].getString("name");
            const std::string key=package+":array/"+name;
            auto it=mArraies.find(key);
            if(it==mArraies.end()) {
                it=mArraies.insert(it,{key,std::vector<std::string>()});
                LOGV("string-array:%s",key.c_str());
            }
            it->second.push_back(value);
        } else if(tags[1].compare("string")==0) {
            const std::string name=atts[1].getString("name");
            const std::string key=package+":string/"+name;
            mStrings[key]=value;
            LOGV("%s=%s",key.c_str(),value.c_str());
        }
    }
}

int Assets::addResource(const std::string&path,const std::string&name) {
    ZIPArchive*pak=new ZIPArchive(path);
    std::string package=name;
    if(name.empty()) {
        size_t pos=path.find_last_of('/');
        if(pos != std::string::npos)
            package = path.substr(pos+1);
        pos = package.find('.');
        if( pos != std::string::npos)
            package = package.substr(0,pos);
    }
    mResources.insert({package,pak});

    int count=0;
    pak->forEachEntry([this,package,&count](const std::string&res) {
        count++;
        if((res.size()>6)&&(TextUtils::startWith(res,"values")||TextUtils::startWith(res,"color"))) {
            LOGV("LoadKeyValues from:%s ...",res.c_str());
            const std::string resid=package+":"+res;
            loadKeyValues(resid,std::bind(&Assets::parseItem,this,package,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
        }
        return 0;
    });
    if(name.compare("cdroid")==0)
        setTheme("cdroid:style/Theme");
    pak->forEachEntry([this,package,&count](const std::string&res) {
        if((res.size()>6)&&TextUtils::startWith(res,"color")) {
            LOGV("LoadKeyValues from:%s:%s ...",package.c_str(),res.c_str());
            std::string resid = package+":"+res.substr(0,res.find(".xml"));
            ColorStateList*cls = ColorStateList::inflate(this,resid);
            mStateColors.insert(std::pair<const std::string,ColorStateList*>(resid,cls));
        }
        return 0;
    });
    LOGD("%s %d resource,[id:%d arraies:%d Styles:%d Strings:%d]",name.c_str(),count,mIDS.size(),mArraies.size(),mStyles.size(),mStrings.size());
    return pak?0:-1;
}

static bool guessExtension(ZIPArchive*pak,std::string&ioname) {
    bool ret=(ioname.find('.')!=std::string::npos);
    if(ret)return ret;
    if(TextUtils::startWith(ioname,"mipmap")) {
        if(ret=pak->hasEntry(ioname+".png",false)) ioname+=".png";
        else if(ret=pak->hasEntry(ioname+".9.png",false))ioname+=".9.png";
        else if(ret=pak->hasEntry(ioname+".jpg",false))ioname+=".jpg";
    } else {
        if(ret=pak->hasEntry(ioname+".xml",false))
            ioname+=".xml";
    }
    return ret;
}

//"@[package:][+]id/filname"
const std::string Assets::parseResource(const std::string&fullResId,std::string*res,std::string*ns)const {
    std::string pkg = mName;
    std::string relname= fullResId;
    std::string fullid = fullResId;

    size_t pos=fullid.find_last_of("@+");
    if(pos!=std::string::npos)fullid =fullid.erase(0,pos+1);

    pos= fullid.find(":");
    if(pos != std::string::npos) {
        pkg = fullid.substr(0,pos);
        relname= fullid.substr(pos+1);
    } else { //id/xxx
        pos = mName.find_last_of('/');
        if(pos != std::string::npos)
            pkg = mName.substr(pos+1);
        relname = fullid;
    }
    if(pkg=="android") pkg="cdroid";
    if( ns) *ns = pkg;
    if(res)*res= relname;
    return pkg+":"+relname;
}

ZIPArchive*Assets::getResource(const std::string&fullResId,std::string*relativeResID,std::string*outPackage)const {
    std::string package,resname;
    parseResource(fullResId,&resname,&package);
    auto it = mResources.find(package);
    ZIPArchive* pak = nullptr;
    if(outPackage) *outPackage = package;
    if(it != mResources.end()) { //convert noextname ->extname.
        pak = it->second;
        guessExtension(pak,resname);
        if(relativeResID) *relativeResID = resname;
    }
    LOGV_IF(pak==nullptr && resname.size(),"resource for [%s] is%s found",fullResId.c_str(),(pak?"":" not"));
    return pak;
}

std::unique_ptr<std::istream> Assets::getInputStream(const std::string&fullresid,std::string*outpkg) {
    std::string resname;
    ZIPArchive*pak = getResource(fullresid,&resname,outpkg);
    if(pak){
	std::istream*stream = pak->getInputStream(resname);
	if(stream)return std::unique_ptr<std::istream>(stream);
    }
    return std::make_unique<std::ifstream>(fullresid);
}

void Assets::loadStrings(const std::string&lan) {
    for(auto a:mResources) {
        const std::string fname = "strings/strings-"+lan+".xml";
        loadKeyValues(fname,std::bind(&Assets::parseItem,this,a.first,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));
    }
}

static Cairo::RefPtr<Cairo::ImageSurface> LoadImage( std::istream&istream ){
    return Cairo::ImageSurface::create_from_stream(istream);
}

RefPtr<ImageSurface>Assets::loadImage(const std::string&fullresid) {
    size_t capacity = 0;
    std::string resname;
    ZIPArchive*pak = getResource(fullresid,&resname,nullptr);
    if(pak)guessExtension(pak,resname);
    void*zfile = pak ? pak->getZipHandle(resname):nullptr;
    ZipInputStream zipis(zfile);
    RefPtr<ImageSurface>img;
    if(zfile==nullptr) {
        std::ifstream fi(fullresid);
        img = LoadImage(fi);
        LOGD_IF(img==nullptr,"pak=%p %s open failed",pak,resname.c_str());
        return img;
    }else if(zfile){
        img = LoadImage(zipis);
    }
    LOGV_IF(img,"image %s size=%dx%d",fullresid.c_str(),img->get_width(),img->get_height());
    return img;
}

Cairo::RefPtr<Cairo::ImageSurface> Assets::loadImage(const std::string&resname,int width,int height,int scaleType){
    return nullptr;
}

int Assets::getId(const std::string&resname)const {
    std::string resid,pkg;
    std::string key = resname;
    if(key.empty())return -1;
    if(key.length()&&(key.find('/')==std::string::npos))
        return TextUtils::strtol(key);
    auto pos =key.find('+');
    if(pos!=std::string::npos)
        key.erase(pos,1);
    parseResource(key,&resid,&pkg);

    auto it=mIDS.find(pkg+":"+resid);
    return it==mIDS.end()?-1:it->second;
}

int Assets::getNextAutofillId(){
    return mNextAutofillViewId++;
}

const std::string Assets::getString(const std::string& resid,const std::string&lan) {
    if((!lan.empty())&&(mLanguage!=lan)) {
        loadStrings(lan);
    }
    std::string str = resid;
    std::string pkg,name=resid;
    parseResource(resid,&name,&pkg);
    name = AttributeSet::normalize(pkg,resid);
    auto itr = mStrings.find(name);
    if(itr != mStrings.end()) {
        str = itr->second;
    }
    TextUtils::replace(str,"\\n","\n");
    return str;
}

int Assets::getArray(const std::string&resid,std::vector<int>&out) {
    std::string pkg,name = resid;
    std::string fullname = parseResource(resid,&name,&pkg);
    auto it = mArraies.find(fullname);
    if(it != mArraies.end()) {
        for(auto itm:it->second)
           out.push_back(std::stoi(itm));
        return out.size();
    }
    return  0;
}

int Assets::getArray(const std::string&resid,std::vector<std::string>&out) {
    std::string pkg,name = resid;
    std::string fullname=parseResource(resid,&name,&pkg);
    auto it = mArraies.find(fullname);
    if(it != mArraies.end()) {
        for(auto itm:it->second){
            itm = AttributeSet::normalize(pkg,itm);
            out.push_back(itm);
        }
        return out.size();
    }
    ZIPArchive * pak = getResource(resid,&name,nullptr);
    pak->forEachEntry([&out,pkg](const std::string&res){
        if(TextUtils::startWith(res,"font")){
            std::string fullres = AttributeSet::normalize(pkg,res);
            out.push_back(fullres);
        }
        return out.size();
    });
    return 0;
}


Drawable* Assets::getDrawable(const std::string&fullresid) {
    Drawable* d = nullptr;
    std::string resname,package;
    ZIPArchive* pak = getResource(fullresid,&resname,&package);
    if(fullresid.empty()||(fullresid.compare("null")==0)) {
        return d;
    } else {
        auto it = mDrawables.find(fullresid);
        if( it != mDrawables.end() ) {
            if(it->second.expired()==false) {
                auto cs=it->second.lock();
                d= cs->newDrawable();
                LOGV("%s:%p use_count=%d",fullresid.c_str(),d,it->second.use_count());
                return d;
            }
            mDrawables.erase(it);
        }
    }
    //wrap png to drawable,make app develop simply
    if(resname[0]=='#'||resname[1]=='x'|| resname[1]=='X'){
        LOGV("color %s",fullresid.c_str());
        d = new ColorDrawable(Color::parseColor(resname));
        mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
        return d;
    }
    if(resname.find("color/")!=std::string::npos){
        auto itc = mColors.find(fullresid);
        auto its = mStateColors.find(fullresid);
        if(itc!=mColors.end()){
	    const uint32_t cc = (uint32_t)getColor(fullresid);
            LOGV("%s use colors as drawable",fullresid.c_str());
            d = new ColorDrawable(cc);
            mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
            return d;
        } else if(its!=mStateColors.end()){
            LOGV("%s use colorstatelist as drawable",fullresid.c_str());
            d = new StateListDrawable(*its->second);
            mDrawables.insert(std::pair<std::string,std::weak_ptr<Drawable::ConstantState>>(fullresid,d->getConstantState()));
            return d;
        }
    }

    if(resname.find("attr/")!=std::string::npos) {//for reference resource
        resname = mTheme.getString(resname.substr(5));
        d = getDrawable(resname);
    } else if(resname.find("color/")!=std::string::npos) {
        const uint32_t cc = (uint32_t)getColor(fullresid);
        return new ColorDrawable(cc);
    } else if(TextUtils::endWith(resname,".9.png")) {
        d = new NinePatchDrawable(this,fullresid);
    } else if (TextUtils::endWith(resname,".png")||TextUtils::endWith(resname,".jpg")) {
        d = new BitmapDrawable(this,fullresid);
    } else if(TextUtils::endWith(resname,".gif")||TextUtils::endWith(resname,".apng")
		    ||TextUtils::endWith(resname,".webp")){
        d = new AnimatedImageDrawable(this,fullresid);
    }
    if( (d == nullptr) && (!fullresid.empty()) ) {
        void*zfile = pak ? pak->getZipHandle(resname) : nullptr;
        if(zfile) {
            ZipInputStream zs(zfile);
            d = Drawable::fromStream(this,zs,resname,package);
        } else if(!resname.empty()) {
            std::ifstream fs(fullresid);
            d = Drawable::fromStream(nullptr,fs,resname,package);
        }
        LOGD_IF(zfile==nullptr&&fullresid.find("/")!=std::string::npos,"drawable %s load failed",fullresid.c_str());
    }
    if(d) {
        mDrawables.insert({fullresid,std::weak_ptr<Drawable::ConstantState>(d->getConstantState())});
    }
    return d;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
//codes between pragma will crashed in ubuntu GCC V8.x,bus GCC V7 wroked well.
int Assets::getColor(const std::string&refid) {
    std::string pkg,name = refid;
    parseResource(name,nullptr,&pkg);
    name = AttributeSet::normalize(pkg,name);
    auto it = mColors.find(name);
    if(it != mColors.end()) {
        return it->second;
    } else if((refid[0]=='#')||refid.find(':')==std::string::npos) {
        return Color::parseColor(refid);
    } else if(refid.find("color/")==std::string::npos) { //refid is defined as an color reference
        parseResource(refid,&name,nullptr);
        name = mTheme.getString(name);
        return getColor(name);
    }
    return 0xFF000000;
}

ColorStateList* Assets::getColorStateList(const std::string&fullresid) {
    std::string pkg,name = fullresid;
    parseResource(name,nullptr,&pkg);
    name = AttributeSet::normalize(pkg,name);
    auto itc= mColors.find(name);
    auto its = mStateColors.find(name);
    if( its!=mStateColors.end())
        return its->second;
    else if(itc != mColors.end()){
        ColorStateList* cls = new ColorStateList(itc->second);
        mStateColors.insert(std::pair<const std::string,ColorStateList*>(fullresid,cls));
        return cls;
    }else if( (itc == mColors.end()) && (name.empty()==false) ) {
        size_t slashpos = fullresid.find("/");
        if( (fullresid[0]=='#') || (slashpos==std::string::npos) ) {/*digital colors*/
            const int color = Color::parseColor(fullresid);
            ColorStateList* cls = new ColorStateList(color);
            mStateColors.insert(std::pair<const std::string,ColorStateList*>(fullresid,cls));
            return cls;
        }
        if( slashpos==std::string::npos ) {/*for color wolrds*/
            std::string realName;
            parseResource(fullresid,&realName,nullptr);
            realName = mTheme.getString(realName);
            itc = mColors.find(realName);
            if(itc != mColors.end()){
                ColorStateList* cls = new ColorStateList(itc->second);
                mStateColors.insert(std::pair<const std::string,ColorStateList*>(fullresid,cls));
                return cls;
            }
        }
    } else if(fullresid.find("attr")!=std::string::npos) {
        size_t slashpos=fullresid.find("/");
        std::string name = fullresid.substr(slashpos+1);
        name = mTheme.getString(name);
        if(!name.empty())return getColorStateList(name);
    }
    LOGD_IF(!fullresid.empty(),"%s not found",fullresid.c_str());
    return nullptr;
}

typedef struct {
    std::vector<std::string> tags;
    std::vector<AttributeSet> attrs;
    std::string content;
    std::function<void(const std::vector<std::string>&,const std::vector<AttributeSet>&attrs,const std::string&)>func;
} KVPARSER;

static void startElement(void *userData, const XML_Char *name, const XML_Char **satts) {
    KVPARSER* kvp = (KVPARSER*)userData;
    if(strcmp(name,"resources")) { //root node is not in KVPARSER::attrs
        AttributeSet atts;
        atts.set(satts);
        kvp->tags.push_back(name);
        kvp->attrs.push_back(atts);
        kvp->content = std::string();
    }
}

static void CharacterHandler(void *userData,const XML_Char *s, int len) {
    KVPARSER*kvp = (KVPARSER*)userData;
    kvp->content+=std::string(s,len);
}

static void endElement(void *userData, const XML_Char *name) {
    KVPARSER*kvp = (KVPARSER*)userData;
    if(strcmp(name,"resources")) { //root node is not in KVPARSER::attrs
	//we need to keep whitespace in xml's textarea,so we cant strip space(TextUtils::trim(kvp->content)
        kvp->func(kvp->tags,kvp->attrs,kvp->content);
        kvp->attrs.pop_back();
        kvp->tags.pop_back();
        kvp->content=std::string();
    }
}

int Assets::loadKeyValues(const std::string&fullresid,std::function<void(const std::vector<std::string>&,
                          const std::vector<AttributeSet>&atts,const std::string&)>func) {
    int len = 0;
    char buf[256];

    std::unique_ptr<std::istream>stream = getInputStream(fullresid);
    LOGE_IF(stream == nullptr,"%s load failed",fullresid.c_str());
    if(stream == nullptr)
        return 0;
    XML_Parser parser = XML_ParserCreate(nullptr);
    std::string curKey;
    std::string curValue;
    KVPARSER kvp;
    kvp.func = func;
    XML_SetUserData(parser,&kvp);
    XML_SetElementHandler(parser, startElement, endElement);
    XML_SetCharacterDataHandler(parser,CharacterHandler);
    do {
        stream->read(buf,sizeof(buf));
        len = stream->gcount();
        if (XML_Parse(parser, buf,len,len==0) == XML_STATUS_ERROR) {
            const char*es=XML_ErrorString(XML_GetErrorCode(parser));
            LOGE("%s:%s at line %ld",fullresid.c_str(),es, XML_GetCurrentLineNumber(parser));
            XML_ParserFree(parser);
            return 0;
        }
    } while(len!=0);
    XML_ParserFree(parser);
    return 0;
}
#pragma GCC pop_options

void Assets::clearStyles() {
    mStyles.clear();
}

AttributeSet Assets::obtainStyledAttributes(const std::string&resname) {
    AttributeSet atts;
    std::string pkg,name = resname;
    size_t pos = name.find("attr");
    while(pos!=std::string::npos) {
        name = name.replace(pos,4,"style");
        if((pos =name.find('?'))!=std::string::npos)
            name.erase(pos,1);
        if((pos =name.find('/'))!=std::string::npos)
            name=name.substr(pos+1);
        name = mTheme.getString(name);
        if((pos=name.find('@'))!=std::string::npos)
            name.erase(pos,1);
        pos = name.find("attr");
    }
    auto it = mStyles.find(name);
    if(it != mStyles.end())atts = it->second;
    const std::string parent = atts.getString("parent");
    parseResource(name,nullptr,&pkg);
    atts.setContext(this,pkg);
    if(parent.length()) {
        AttributeSet parentAtts = obtainStyledAttributes(parent);
        atts.inherit(parentAtts);
    }
    return atts;
}
}//namespace

