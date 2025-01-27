#include <display.h>
#include <cdgraph.h>
namespace cdroid{

Display::Display(int id,DisplayInfo&displayInfo)
	:mDisplayId(id),mIsValid(true){
   mDisplayInfo = displayInfo;
   mType = TYPE_BUILT_IN;
   displayInfo.type = mType;
}

void Display::updateDisplayInfoLocked(){
}

int Display::getDisplayId(){
    updateDisplayInfoLocked();
    return mDisplayId;
}

int Display::getType(){
    return mType;
}

bool Display::isValid(){
    return mIsValid;
}

bool Display::getDisplayInfo(DisplayInfo&info){
    updateDisplayInfoLocked();
    info=mDisplayInfo;
    return true;
}

void Display::getSize(Point&outSize){
    updateDisplayInfoLocked();
    GFXGetDisplaySize(mDisplayId,(UINT*)&outSize.x,(UINT*)&outSize.y);
    const int rotation = mDisplayInfo.rotation;
    if((rotation==ROTATION_90)||(mDisplayInfo.rotation==ROTATION_270))
        std::swap(outSize.x,outSize.y);
}

void Display::getRealSize(Point&outSize){
    updateDisplayInfoLocked();
    GFXGetDisplaySize(mDisplayId,(UINT*)&outSize.x,(UINT*)&outSize.y);
}

int Display::getRotation(){
    updateDisplayInfoLocked();
    return mDisplayInfo.rotation;
}

void Display::getMetrics(DisplayMetrics&outMetrics){
    GFXGetDisplaySize(mDisplayId,(UINT*)&outMetrics.widthPixels,(UINT*)&outMetrics.heightPixels);
}

void Display::getRealMetrics(DisplayMetrics&outMetrics){
    GFXGetDisplaySize(mDisplayId,(UINT*)&outMetrics.widthPixels,(UINT*)&outMetrics.heightPixels);
}

}
