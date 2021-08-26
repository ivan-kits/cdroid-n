#ifndef __PROGRESS_BAR_H__
#define __PROGRESS_BAR_H__
#include <widget/view.h>
#include <animation/objectanimator.h>
#include <animation/alphaanimation.h>

namespace cdroid{

class ProgressBar:public View{
private:
    static constexpr int MAX_LEVEL = 10000;
    static constexpr int PROGRESS_ANIM_DURATION = 80;
    struct RefreshData{
        int id;
        int progress;
        bool fromUser;
        bool animate;
        RefreshData(){
            id=0;progress=0;
            animate=false;
            fromUser=false;
        }
    };
    
    bool mAttached;
    float mVisualProgress;
    int mBehavior;
    int mDuration;
    bool mOnlyIndeterminate;
    Transformation* mTransformation;
    AlphaAnimation* mAnimation;
    bool mHasAnimation;
    bool mInDrawing;
    bool mRefreshIsPosted;
    std::vector<RefreshData>mData;
    bool mShouldStartAnimationDrawable;
    class ProgressTintInfo*mProgressTintInfo;
    ObjectAnimator*mAnimator;

    bool mNoInvalidate;
    Interpolator* mInterpolator;

    void initProgressBar();
    void swapCurrentDrawable(Drawable*d);
    void updateDrawableBounds(int width,int height);
    void updateDrawableState();
    void setVisualProgress(int id, float progress);

    void applyProgressTints();
    void applyPrimaryProgressTint();
    void applyProgressBackgroundTint();
    void applySecondaryProgressTint();
protected:
    enum{//progress id
        ID_BACKGROUND=0,
        ID_PRIMARY=1,
        ID_SECONDARY =2
    };
    int mMin;
    int mMax;
    int mMinWidth;
    int mMaxWidth;
    int mMinHeight;
    int mMaxHeight;
    int mProgress;
    int mSecondaryProgress;
    int  indeterminatePos;
    bool mIndeterminate;
    bool mMirrorForRtl;
    Drawable*mCurrentDrawable;
    Drawable*mProgressDrawable;
    Drawable*mIndeterminateDrawable;
    void startAnimation();
    void stopAnimation();
    Drawable*getCurrentDrawable()const;
    void drawableStateChanged()override;
    bool verifyDrawable(Drawable* who)const override;
    virtual void drawTrack(Canvas&canvas);
    void refreshProgress(int id, int progress, bool from,bool animate);
    void doRefreshProgress(int id, int progress, bool fromUser,bool callBackToApp, bool animate);
    void onSizeChanged(int w,int h,int ow,int oh)override;
    virtual bool setProgressInternal(int progress, bool fromUser=false,bool animate=false);
    void onAttached()override;
	void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    virtual void onProgressRefresh(float scale, bool fromUser, int progress){};
    virtual void onVisualProgressChanged(int id, float progress);
    virtual void onDraw(Canvas&canvas)override;
public:
	ProgressBar(Context*ctx,const AttributeSet&attrs);
    ProgressBar(int width, int height);
    ~ProgressBar();
    void setMin(int value);
    void setMax(int value);
    int getMin()const {return mMin;}
    int getMax()const {return mMax;}
    void setRange(int vmin,int vmax);

    void setProgress(int value);
    int getProgress()const;
    void setSecondaryProgress(int secondaryProgress);
    int getSecondaryProgress()const;

    void incrementProgressBy(int diff);
    void incrementSecondaryProgressBy(int diff);
    bool isIndeterminate()const;
    void setIndeterminate(bool indeterminate);

    void setProgressDrawable(Drawable*d);
    Drawable*getProgressDrawable()const;
    void setIndeterminateDrawable(Drawable*d);
    Drawable*getIndeterminateDrawable()const;

    void setProgressTintList(ColorStateList*tint);
    ColorStateList*getProgressTintList()const;
    void setProgressTintMode(int);
    int getProgressTintMode()const;
    void setProgressBackgroundTintList(ColorStateList*tint);
    ColorStateList*getProgressBackgroundTintList()const;
    void setProgressBackgroundTintMode(int);
    int getProgressBackgroundTintMode()const;
    void setSecondaryProgressTintList(ColorStateList*tintt);
    ColorStateList*getSecondaryProgressTintList()const;
    void setSecondaryProgressTintMode(int tintMode);
    int getSecondaryProgressTintMode()const;
    Drawable* getTintTarget(int layerId, bool shouldFallback);
    void setProgressDrawableTiled(Drawable* d);
};

}
#endif
