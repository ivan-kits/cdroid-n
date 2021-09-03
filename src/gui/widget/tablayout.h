#pragma once
#include <widget/horizontalscrollview.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <widget/imageview.h>
#include <widget/viewpager.h>

namespace cdroid{

class TabLayout:public HorizontalScrollView{
public:
    static constexpr int MODE_SCROLLABLE = 0;
    static constexpr int MODE_FIXED = 1;
    static constexpr int GRAVITY_FILL = 0;
    static constexpr int GRAVITY_CENTER = 1;
    static constexpr int INDICATOR_GRAVITY_BOTTOM = 0;
    static constexpr int INDICATOR_GRAVITY_CENTER = 1;
    static constexpr int INDICATOR_GRAVITY_TOP = 2;
    static constexpr int INDICATOR_GRAVITY_STRETCH = 3;
    class Tab{
    private:
        void* mTag;
        Drawable* mIcon;
        std::string mText;
        std::string  mContentDesc;
        int mPosition = INVALID_POSITION;
        View* mCustomView;
    public:
        static constexpr int INVALID_POSITION = -1;
        TabLayout* mParent;
        View* mView;/*TabView*/

        Tab();
        ~Tab(); 
        void*getTag()const;
        void setTag(void*tag);
        View* getCustomView()const;
        Tab& setCustomView(View*);
        Drawable* getIcon()const;
        Tab& setIcon(Drawable* icon);
        int getPosition()const;
        void setPosition(int position);
        std::string getText()const;
        Tab& setText(const std::string&text);
        void select();
        bool isSelected()const;
        std::string getContentDescription()const;
        Tab& setContentDescription(const std::string&contentDesc);
        void updateView();
        void reset();
    };

    class TabView:public LinearLayout{
    private:
        static constexpr int CUSTOM_ID_TEXT =0;
        static constexpr int CUSTOM_ID_ICON =1;
        TabLayout*mParent;
        Tab* mTab;
        TextView* mTextView;
        ImageView* mIconView;

        View* mCustomView;
        TextView* mCustomTextView;
        ImageView* mCustomIconView;

        int mDefaultMaxLines = 2;
 
        void updateTextAndIcon(TextView* textView,ImageView* iconView);
        float approximateLineWidth(Layout* layout, int line, float textSize);
    public:
        TabView(Context* context,const AttributeSet&atts,TabLayout*parent);
        bool performClick()override;
        void setSelected(bool selected)override;
        void onMeasure(int origWidthMeasureSpec,int origHeightMeasureSpec)override;
        void setTab(Tab* tab);
        void reset();
        void update();
        Tab* getTab();
    };

    struct OnTabSelectedListener{
       CallbackBase<void,Tab&>onTabSelected;
       CallbackBase<void,Tab&>onTabUnselected;
       CallbackBase<void,Tab&>onTabReselected;
    };

    class TabItem:public View{
    public:
       std::string mText;
       Drawable* mIcon;
       std::string mCustomLayout;
       TabItem();
       TabItem(Context* context,const AttributeSet& attrs);
    };
private:
    class SlidingTabStrip:public LinearLayout{
    private:
        int mSelectedIndicatorHeight;
        int mSelectedIndicatorColor;

        int mSelectedPosition= -1;
        float mSelectionOffset;

        int mLayoutDirection = -1;

        int mIndicatorLeft  = -1;
        int mIndicatorRight = -1;
        TabLayout*mParent;
        ValueAnimator* mIndicatorAnimator;
        void updateIndicatorPosition();
    protected:
        void onMeasure(int widthMeasureSpec,int heightMeasureSpec)override;
        void onLayout(bool changed, int l, int t, int r, int b);
    public:
        SlidingTabStrip(Context* context,const AttributeSet&atts,TabLayout*parent);
        void setSelectedIndicatorColor(int color);
        void setSelectedIndicatorHeight(int height);
        bool childrenNeedLayout();
        void setIndicatorPositionFromTabPosition(int position, float positionOffset);
        float getIndicatorPosition();
        void onRtlPropertiesChanged(int layoutDirection)override;
        void setIndicatorPosition(int left, int right);
        void animateIndicatorToPosition(int position, int duration);
        void draw(Canvas& canvas)override;
    };
    int mRequestedTabMinWidth;
    int mRequestedTabMaxWidth;
    int mScrollableTabMinWidth;
    int mContentInsetStart;

    ValueAnimator* mScrollAnimator;
    Tab* mSelectedTab;
    SlidingTabStrip* mTabStrip;
    bool mSetupViewPagerImplicitly;

    void initTabLayout();
    void addTabFromItemView(TabItem* item);
    void setupWithViewPager(ViewPager* viewPager, bool autoRefresh, bool implicitSetup);
    int getTabScrollRange();
    void updateAllTabs();
    TabView*createTabView(Tab* tab);
    void configureTab(Tab* tab, int position);
    void addTabView(Tab* tab);
    void removeTabViewAt(int position);
    void animateToTab(int newPosition);
    void ensureScrollAnimator();
    void setScrollAnimatorListener(Animator::AnimatorListener listener);
    void setSelectedTabView(int position);
    void dispatchTabSelected(Tab* tab);
    void dispatchTabUnselected(Tab* tab);
    void dispatchTabReselected(Tab* tab);
    int calculateScrollXForTab(int position, float positionOffset);
    void applyModeAndGravity();

    static ColorStateList* createColorStateList(int defaultColor, int selectedColor);
    int getDefaultHeight();
    int getTabMinWidth();
protected:
    static constexpr int FIXED_WRAP_GUTTER_MIN = 16; //dps
    static constexpr int MOTION_NON_ADJACENT_OFFSET = 24;
    int mTabPaddingStart;
    int mTabPaddingTop;
    int mTabPaddingEnd;
    int mTabPaddingBottom;
    int mTabTextAppearance;
    ColorStateList* mTabTextColors;
    ColorStateList* mTabIconTint;
    ColorStateList* mTabRippleColorStateList;
    Drawable* mTabSelectedIndicator;
    float mTabTextSize;
    float mTabTextMultiLineSize;
    int mTabBackgroundResId;
    int mTabMaxWidth;

    int mTabGravity;
    int mTabIndicatorAnimationDuration;
    int mTabIndicatorGravity;
    int mMode;
    std::vector<OnTabSelectedListener> mSelectedListeners;
    OnTabSelectedListener mCurrentVpSelectedListener;
    bool inlineLabel;
    bool tabIndicatorFullWidth;
    bool unboundedRipple;
    ViewPager* mViewPager;
    std::vector<Tab*>mTabs;

    void setScrollPosition(int position, float positionOffset, bool updateSelectedText, bool updateIndicatorPosition);

    void setPagerAdapter(PagerAdapter* adapter,bool addObserver);
    void populateFromPagerAdapter();
    View& addViewInternal(View* child);
    LinearLayout::LayoutParams* createLayoutParamsForTabs();
    void updateTabViewLayoutParams(LinearLayout::LayoutParams* lp);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void selectTab(Tab* tab,bool updateIndicator);
    void updateTabViews(bool requestLayout);
    int getTabMaxWidth();
public:
    TabLayout(int w,int h);
    TabLayout(Context*context,const AttributeSet&atts);
    void setSelectedTabIndicatorColor( int color);
    void setSelectedTabIndicatorHeight(int height);
    void setScrollPosition(int position, float positionOffset, bool updateSelectedText);
    float getScrollPosition()const;
    void addTab(Tab* tab);
    void addTab(Tab* tab, int position);
    void addTab(Tab* tab, bool setSelected);
    void addTab(Tab* tab, int position, bool setSelected);

    void addOnTabSelectedListener(OnTabSelectedListener listener);
    void removeOnTabSelectedListener(OnTabSelectedListener listener);
    void clearOnTabSelectedListeners();
    Tab* newTab();
    int getTabCount()const;
    Tab* getTabAt(int index);
    int getSelectedTabPosition()const;
    void removeTab(Tab* tab);
    void removeTabAt(int position);
    void removeAllTabs();
    void setTabMode(int mode);
    int getTabMode()const;
    void setTabGravity(int gravity);
    int getTabGravity()const;
    void setTabTextColors(ColorStateList* textColor);
    ColorStateList* getTabTextColors()const;
    void setTabTextColors(int normalColor, int selectedColor);
    void setupWithViewPager(ViewPager* viewPager);
    void setupWithViewPager(ViewPager* viewPager, bool autoRefresh);
    bool shouldDelayChildPressedState()override;

    View& addView(View* child)override;
    View& addView(View* child, int index)override;
    View& addView(View* child, ViewGroup::LayoutParams* params)override;
    View& addView(View* child, int index, ViewGroup::LayoutParams* params)override;

    LayoutParams* generateLayoutParams(const AttributeSet& attrs);
};

}//endof namespace