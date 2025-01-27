#ifndef __DRAWABLE_H__
#define __DRAWABLE_H__
#include <core/canvas.h>
#include <drawables/stateset.h>
#include <drawables/colorstatelist.h>
#include <drawables/colorfilters.h>
#include <core/attributeset.h>
#include <core/context.h>
#include <view/gravity.h>
#include <core/insets.h>
#include <vector>

namespace cdroid{
class ColorStateList;
class Context;

class Animatable {
public:
    /** Starts the drawable's animation.  */
    virtual void start()=0;
    /** Stops the drawable's animation.  */
    virtual void stop()=0;
    /* Indicates whether the animation is running.
     * @return True if the animation is running, false otherwise.  */
    virtual bool isRunning()=0;
};

class Animatable2:public Animatable{
public:
    typedef struct{
        CallbackBase<void,Drawable&> onAnimationStart;
        CallbackBase<void,Drawable&> onAnimationEnd;
    }AnimationCallback;
    virtual void registerAnimationCallback(AnimationCallback callback)=0;
    virtual bool unregisterAnimationCallback(AnimationCallback callback)=0;
};

enum PixelFormat{
    UNKNOWN=0,
    TRANSLUCENT=1,
    TRANSPARENT=2,
    OPAQUE=3
};

class Drawable{
public:
    class Callback{
    public:
        virtual void invalidateDrawable(Drawable& who)=0;
        virtual void scheduleDrawable(Drawable& who,Runnable what, long when)=0;
        virtual void unscheduleDrawable(Drawable& who,Runnable what)=0;
    };
    class ConstantState{
    public:
        virtual Drawable* newDrawable()=0;
        virtual int getChangingConfigurations()const=0;
		virtual ~ConstantState();
    };
    enum{
        DEFAULT_TINT_MODE=SRC_IN
    };
protected:
    int mLevel;
    bool mVisible;
    int mLayoutDirection;
    int mChangingConfigurations;
    Rect mBounds;
    ColorFilter*mColorFilter;
    Callback*mCallback;
    std::vector<int>mStateSet;
    PorterDuffColorFilter *updateTintFilter(PorterDuffColorFilter* tintFilter,ColorStateList* tint,int tintMode);
    virtual bool onStateChange(const std::vector<int>&) { return false;}
    virtual bool onLevelChange(int level) { return false; }
    virtual bool onLayoutDirectionChanged(int layoutDirection){return false;}
    virtual void onBoundsChange(const Rect& bounds){}
public:
    std::string mResourceId;
public:
    Drawable();
    virtual ~Drawable();
    void setBounds(int x,int y,int w,int h);
    void setBounds(const Rect&r);
    const Rect&getBounds()const;
    virtual Rect getDirtyBounds();
    virtual Drawable*mutate();
    virtual void clearMutated();
    virtual void setColorFilter(ColorFilter*);
    virtual ColorFilter*getColorFilter();
    void setColorFilter(int color,PorterDuffMode mode);
    void clearColorFilter();
    void setTint(int color);
    /*
     * To make memory manager simple,
     * The Drawable must deep copy from the tint,and own the new instance 
     * */
    virtual void setTintList(ColorStateList* tint);
    virtual void setTintMode(int);
    bool setState(const std::vector<int>&state);
    const std::vector<int>& getState()const;
    bool setLevel(int level);
    int getLevel()const{return mLevel;}
    virtual int getOpacity();
    virtual void setHotspot(float x,float y);
    virtual void setHotspotBounds(int left,int top,int width,int height);
    virtual void getHotspotBounds(Rect&outRect);
    virtual bool getPadding(Rect&padding);
    virtual Insets getOpticalInsets();
    virtual bool isStateful()const;
    virtual bool hasFocusStateSpecified()const;
    virtual Drawable*getCurrent();
    virtual void setAlpha(int alpha){};
    virtual int getAlpha()const{return 0xFF;}
    virtual std::shared_ptr<ConstantState>getConstantState();
    virtual void setAutoMirrored(bool mirrored);
    virtual bool isAutoMirrored();
    virtual bool canApplyTheme(){return false;}
    virtual void jumpToCurrentState();

    int getLayoutDirection()const;
    bool setLayoutDirection(int);

    virtual int getIntrinsicWidth() const;
    virtual int getIntrinsicHeight()const;
    virtual int getMinimumWidth() const;
    virtual int getMinimumHeight()const;

    virtual bool setVisible(bool visible, bool restart);
    virtual bool isVisible()const;
    virtual int getChangingConfigurations()const;
    virtual void setChangingConfigurations(int);
    void setCallback(Callback*cbk);
    Callback* getCallback()const;
    void scheduleSelf(Runnable& what, long when);
    void unscheduleSelf(Runnable& what);

    virtual void invalidateSelf();

    virtual void draw(Canvas&ctx)=0;
    static int resolveOpacity(int op1,int op2);
    static float scaleFromDensity(float pixels, int sourceDensity, int targetDensity);
    static int scaleFromDensity(int pixels, int sourceDensity, int targetDensity, bool isSize);
    static Drawable*createWrappedDrawable(Context* ctx,const AttributeSet&atts);
    static Drawable*fromStream(Context*ctx,std::istream&stream,const std::string& basePath=std::string(),
	              const std::string&package=std::string());
    static Drawable*inflate(Context* ctx,const std::string& pathName);
};

}
#endif
