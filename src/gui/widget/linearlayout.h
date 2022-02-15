#ifndef UI_LIBUI_LINEARLAYOUT_H_
#define UI_LIBUI_LINEARLAYOUT_H_

#include <widget/viewgroup.h>

namespace cdroid{

class LinearLayout : public ViewGroup{
public:
    enum{
        HORIZONTAL=0,
        VERTICAL=1,
        SHOW_DIVIDER_NONE=0,
        SHOW_DIVIDER_BEGINNING=1,
        SHOW_DIVIDER_MIDDLE=2,
        SHOW_DIVIDER_END=4
    };
    class LayoutParams:public ViewGroup::MarginLayoutParams{
    public:
        float weight;
        int gravity = -1;
        LayoutParams(Context* c,const AttributeSet&attrs);
        LayoutParams(int width, int height);
        LayoutParams(int width, int height, float weight);
        LayoutParams(const LayoutParams& p);
        LayoutParams(const MarginLayoutParams&source);
        LayoutParams(const ViewGroup::LayoutParams&source);
    };
private:
    bool mAllowInconsistentMeasurement;
    bool mBaselineAligned;
    int mBaselineAlignedChildIndex;
    int mBaselineChildTop;
    int mOrientation;
    int mGravity;
    int mTotalLength;
    float mWeightSum;
    bool mUseLargestChild;
    std::vector<int>mMaxAscent;
    std::vector<int>mMaxDescent;
    int mDividerWidth;
    int mDividerHeight;
    int mShowDividers;
    int mDividerPadding;
    int mLayoutDirection;
    Drawable*mDivider;
    void initView();
    bool isShowingDividers()const;
    bool allViewsAreGoneBefore(int childIndex);
    View* getLastNonGoneChild();
    void forceUniformWidth(int count, int heightMeasureSpec);
    void forceUniformHeight(int count, int widthMeasureSpec);
    void setChildFrame(View* child, int left, int top, int width, int height);
protected:
    virtual int getVirtualChildCount(){return getChildCount();}
    virtual View* getVirtualChildAt(int index){return getChildAt(index);}
    virtual int getChildrenSkipCount(View* child, int index);
    virtual void measureChildBeforeLayout(View* child, int childIndex,int widthMeasureSpec, 
             int totalWidth, int heightMeasureSpec,int totalHeight);
    virtual int getLocationOffset(View* child);
    virtual int getNextLocationOffset(View* child);
    virtual int measureNullChild(int childIndex){return 0;}
    bool hasDividerBeforeChildAt(int childIndex);

    ViewGroup::LayoutParams* generateDefaultLayoutParams()const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    ViewGroup::LayoutParams* generateLayoutParams(const ViewGroup::LayoutParams* lp)const override;

    virtual void onLayout(bool changed, int l, int t, int w, int h);
    void layoutVertical(int left, int top, int width, int height);
    void layoutHorizontal(int left, int top, int width, int height);
    virtual void measureHorizontal(int widthMeasureSpec, int heightMeasureSpec);
    virtual void measureVertical(int widthMeasureSpec, int heightMeasureSpec);
    void drawHorizontalDivider(Canvas& canvas, int top);
    void drawVerticalDivider(Canvas& canvas, int left);
    void drawDividersHorizontal(Canvas& canvas);
    void drawDividersVertical(Canvas& canvas);
    void onDraw(Canvas& canvas)override;
public:
    LinearLayout(int w,int h);
    LinearLayout(int x,int y,int w,int h);
    LinearLayout(Context* context,const AttributeSet& attrs);
    ViewGroup::LayoutParams* generateLayoutParams(const AttributeSet&)const override;
    void setShowDividers(int showDividers);
    int getShowDividers()const;
    Drawable*getDividerDrawable();
    void setDividerDrawable(Drawable*divider);
    void setDividerPadding(int padding);
    int getDividerPadding()const;
    int getDividerWidth()const;
    int getGravity()const;
    void setGravity(int gravity);
    void setHorizontalGravity(int horizontalGravity);
    void setVerticalGravity(int verticalGravity);
    void setOrientation(int orientation);
    int getOrientation()const;
    bool shouldDelayChildPressedState()override;
    void setWeightSum(float weightSum);
    float getWeightSum()const;

    int getBaseline()override;
    bool isBaselineAligned()const;
    void setBaselineAligned(bool baselineAligned);
    bool isMeasureWithLargestChildEnabled()const;
    void setMeasureWithLargestChildEnabled(bool enabled);

    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    virtual ~LinearLayout();
};

}  // namespace cdroid
#endif 
