#include <attributeset.h>
#include <widget/linearlayout.h>
#include <color.h>
#include <gravity.h>
#include <string.h>
#include <vector>
#include <cdlog.h>

namespace cdroid{

static std::vector<std::string> split(const std::string & path) {
    std::vector<std::string> vec;
    size_t begin, end;
    begin = path.find_first_not_of("|");
    while (begin != std::string::npos) {
        end = path.find_first_of("|", begin);
        vec.push_back(path.substr(begin, end-begin));
        begin = path.find_first_not_of("|", end);
    }
    return vec;
}

AttributeSet::AttributeSet(){
   mContext=nullptr;
}

AttributeSet::AttributeSet(const char*atts[],int size){
    set(atts,size);
}

void AttributeSet::setContext(Context*ctx){
    mContext=ctx;
}

int AttributeSet::set(const char*atts[],int size){
    int rc=0;
    for(int i=0;atts[i]&&(size==0||i<size);i+=2,rc+=1){
        const char* key=strrchr(atts[i],' ');
        if(key)key++;else key=atts[i];  
        mAttrs.insert(std::make_pair<const std::string,std::string>(key,atts[i+1]));
    }
    return mAttrs.size();
}

int AttributeSet::inherit(const AttributeSet&other){
    int inheritedCount=0;
    for(auto it=other.mAttrs.begin();it!=other.mAttrs.end();it++){
        if(mAttrs.find(it->first)==mAttrs.end()){
           mAttrs.insert(std::make_pair<const std::string,std::string>(it->first.c_str(),it->second.c_str()));
           inheritedCount++;
        }
    }
    return inheritedCount;
}

bool AttributeSet::add(const std::string&key,const std::string&value){
    if(mAttrs.find(key)!=mAttrs.end())
        return false; 
    mAttrs.insert(std::make_pair<const std::string,std::string>(key.c_str(),value.c_str()));
    return true;
}

bool AttributeSet::hasAttribute(const std::string&key)const{
    return mAttrs.find(key)!=mAttrs.end();
}

int AttributeSet::size()const{
    return mAttrs.size();
}

const std::string AttributeSet::getAttributeValue(const std::string&key)const{
    auto it=mAttrs.find(key);
    if(it!=mAttrs.end())
        return it->second;
    return std::string();
}

bool AttributeSet::getBoolean(const std::string&key,bool def)const{
    const std::string v=getAttributeValue(key);
    if(v.empty())return def;
	return v.compare("true")==0;
}

int AttributeSet::getInt(const std::string&key,int def)const{
    int base=10;
    const std::string v=getAttributeValue(key);
    if(v.empty()||((v[0]>='a')&&(v[0]<='z'))){
        return def;
    }
    if(((v.length()>2)&&(v[1]=='x'||v[1]=='X'))||(v[0]=='#'))
        base=16;
    return std::strtol(v.c_str(),nullptr,base);
}

int AttributeSet::getInt(const std::string&key,const std::map<const std::string,int>&kvs,int def)const{
    const std::string vstr=getAttributeValue(key);
    if(vstr.size()&&vstr.find('|')!=std::string::npos){
        std::vector<std::string>gs=split(vstr);
        int result=0;
        int count=0;
        for(std::string s:gs){
            auto it=kvs.find(s);
            if(it!=kvs.end()){
                result|=it->second;
                count++;
            }
        }
        return count?result:def;
    }else{
        auto it=kvs.find(vstr);
        return it==kvs.end()?def:it->second;
    }
}

int AttributeSet::getResourceId(const std::string&key,int def)const{
    return getInt(key,def);
}

int AttributeSet::getColor(const std::string&key,int def)const{
    const std::string v=getAttributeValue(key);
    if(v.empty())return def;
    return Color::parseColor(v);
}

float AttributeSet::getFloat(const std::string&key,float def)const{
    const std::string v=getAttributeValue(key);
    if(v.empty())return def;
    return std::strtof(v.c_str(),nullptr);
}

float AttributeSet::getFraction(const std::string&key,int base,int pbase,float def)const{
    const std::string v=getAttributeValue(key);
    if(v.empty())return def;
    float ret=std::strtof(v.c_str(),nullptr);
    if(v.find('%')!=std::string::npos)ret/=100.f;
    return ret;
}

const std::string AttributeSet::getString(const std::string&key,const std::string&def)const{
    const std::string v=getAttributeValue(key);
    if(v.empty())return def;
    return v;
}

static std::map<const std::string,int>gravitykvs={
    {"top"   , Gravity::TOP}   ,
    {"bottom", Gravity::BOTTOM},    
    {"left"  , Gravity::LEFT}  ,   
    {"right" , Gravity::RIGHT} ,
    {"center_vertical"  , Gravity::CENTER_VERTICAL},
    {"fill_vertical"    , Gravity::FILL_VERTICAL}  ,
    {"center_horizontal", Gravity::CENTER_HORIZONTAL},
    {"fill_horizontal"  , Gravity::FILL_HORIZONTAL}  ,
    {"center", Gravity::CENTER},
    {"fill"  , Gravity::FILL}  ,
    {"clip_vertical"  , Gravity::CLIP_VERTICAL},
    {"clip_horizontal", Gravity::CLIP_HORIZONTAL}
};

int AttributeSet::getGravity(const std::string&key,int defvalue)const{
    //return getInt(key,gravitykvs,defvalue);
    int gravity=0;
    const std::string prop=getString(key);
    std::vector<std::string>gs=split(prop);
    for(auto s:gs){
        auto it=gravitykvs.find(s);
        if(it!=gravitykvs.end()){
            gravity|=it->second;
        }
    }
    return gs.size()?gravity:defvalue;
}

int AttributeSet::getDimensionPixelSize(const std::string&key,int def)const{
    const std::string v=getString(key);
    if(v.empty())return def;
    return std::strtol(v.c_str(),nullptr,10);
}

int AttributeSet::getDimensionPixelOffset(const std::string&key,int def)const{
    return getDimensionPixelSize(key,def);
}

int AttributeSet::getLayoutDimension(const std::string&key,int def)const{
    const std::string v=getString(key);
    if(v.empty())return def;
    switch(v[0]){
    case 'f':
    case 'm':return -1;//MATCH_PARENT
    case 'w':return -2;//WRAP_CONTENT
    default :return std::strtol(v.c_str(),nullptr,10);
    }
}
void AttributeSet::dump()const{
    for(auto it=mAttrs.begin();it!=mAttrs.end();it++){
       LOGD("%s:%s",it->first.c_str(),it->second.c_str());
    }
}

}
