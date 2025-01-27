#ifndef __ABS_LISTVIEW_H__
#define __ABS_LISTVIEW_H__
#include <core/sparsearray.h>
#include <widget/adapterview.h>
#include <widget/recyclebin.h>
#include <widget/overscroller.h>
#include <widget/edgeeffect.h>
#include <widget/popupwindow.h>
#include <widget/filterable.h>
namespace cdroid{

#define OVERSCROLL_LIMIT_DIVISOR 3
#define CHECK_POSITION_SEARCH_DISTANCE 20

class AbsListView:public AdapterView,Filter::FilterListener{
public:
    friend RecycleBin;
    enum ChoiceMode{
        CHOICE_MODE_NONE=0,
        CHOICE_MODE_SINGLE=1,
        CHOICE_MODE_MULTIPLE=2,
        CHOICE_MODE_MULTIPLE_MODAL=3
    };
    enum TouchMode{
        TOUCH_MODE_REST=-1,
        TOUCH_MODE_DOWN=0,
        TOUCH_MODE_TAP=1,
        TOUCH_MODE_DONE_WAITING=2,
        TOUCH_MODE_SCROLL=3,
        TOUCH_MODE_FLING=4,
        TOUCH_MODE_OVERSCROLL=5,
        TOUCH_MODE_OVERFLING =6
    };
    enum{
        TRANSCRIPT_MODE_DISABLED=0,
        TRANSCRIPT_MODE_NORMAL=1,
        TRANSCRIPT_MODE_ALWAYS_SCROLL=2
    };
    enum{
        LAYOUT_NORMAL =0,
        LAYOUT_FORCE_TOP =1,
        LAYOUT_SET_SELECTION =2,
        LAYOUT_FORCE_BOTTOM =3,
        LAYOUT_SPECIFIC =4,
        LAYOUT_SYNC =5,
        LAYOUT_MOVE_SELECTION =6
    };
    class LayoutParams:public ViewGroup::LayoutParams{
    private:
        void init();
    public:
        int viewType;
        bool recycledHeaderFooter;
        bool forceAdd;
        int scrappedFromPosition;
        long itemId ;
        bool isEnabled;
        LayoutParams();
        LayoutParams(const ViewGroup::LayoutParams&);
        LayoutParams(int w,int h);
        LayoutParams(int w,int h,int vt);
        LayoutParams(Context*ctx,const AttributeSet&atts);
    };
    struct OnScrollListener{
        static constexpr int SCROLL_STATE_IDLE=0;
        static constexpr int SCROLL_STATE_TOUCH_SCROLL =1;
        static constexpr int SCROLL_STATE_FLING =2;
        // void onScrollStateChanged(AbsListView view, int scrollState)
        std::function<void(AbsListView&view,int)>onScrollStateChanged;
        //void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount,int totalItemCount);
        std::function<void(AbsListView&view,int,int,int)>onScroll;
    }; 
    DECLARE_UIEVENT(void,MultiChoiceModeListener,/*ActionMode mode,*/int position, long id, bool checked);
protected:
    class AbsPositionScroller:public Runnable {
    public:
        virtual void start(int position)=0;
        virtual void start(int position, int boundPosition)=0;
        virtual void startWithOffset(int position, int offset)=0;
        virtual void startWithOffset(int position, int offset, int duration)=0;
        virtual void stop()=0;
    };
    class PositionScroller:public AbsPositionScroller{
    private:
        static constexpr int SCROLL_DURATION = 200;    
        static constexpr int MOVE_DOWN_POS = 1;
        static constexpr int MOVE_UP_POS = 2;
        static constexpr int MOVE_DOWN_BOUND = 3;
        static constexpr int MOVE_UP_BOUND = 4;
        static constexpr int MOVE_OFFSET = 5;
    
        int mMode;
        int mTargetPos;
        int mBoundPos;
        int mLastSeenPos;
        int mScrollDuration;
        int mExtraScroll;
        int mOffsetFromTop;
        AbsListView*mLV;
    private:
        void scrollToVisible(int targetPos, int boundPos, int duration);
    public:
        PositionScroller(AbsListView*);
        void start(int position)override;
        void start(int position, int boundPosition)override;
        void startWithOffset(int position, int offset)override;
        void startWithOffset(int position, int offset, int duration)override;
        void stop()override;
        void operator()();
    };
private:

    class AbsRunnable:public Runnable{
    protected:
        AbsListView*mLV;
    public:
        AbsRunnable();
        virtual void setList(AbsListView*); 
        virtual void run()=0;
    };
    class CheckForTap:public AbsRunnable{
    public:
        float x,y;
        void run()override;
    };
    class WindowRunnable:public AbsRunnable{
    protected:
        int mOriginalAttachCount;
    public:
        WindowRunnable();
        void rememberWindowAttachCount();
        bool sameWindow();
    };
    class PerformClick:public WindowRunnable{
    public:
        int mClickMotionPosition;
        void run()override;
    };
    class CheckForLongPress:public WindowRunnable{
    private:
        static constexpr int INVALID_COORD=-1;
        float mX ,mY;
    public:
        void setCoords(float x, float y);
        void run()override;
    };
    class CheckForKeyLongPress:public WindowRunnable{
    public:
        void run()override;
    };
    class FlingRunnable:public AbsRunnable{
    public:
        static constexpr int FLYWHEEL_TIMEOUT = 40;
        OverScroller* mScroller;
        int mLastFlingY;
        bool mSuppressIdleStateChangeCall;
        Runnable mCheckFlywheel;
        void checkFlyWheel();
    public:
        FlingRunnable();
        ~FlingRunnable()override;
        void setList(AbsListView*)override;
        void start(int initialVelocity);
        void startSpringback();
        void startOverfling(int initialVelocity);
        void edgeReached(int delta);
        void startScroll(int distance, int duration, bool linear,
                bool suppressEndFlingStateChangeCall);
        void endFling();
        void flywheelTouch();
        void run()override;
    };
private:
    constexpr static int FLYWHEEL_TIMEOUT =40;
    constexpr static bool PROFILE_SCROLLING = false;
    static const bool PROFILE_FLINGING = true;
    enum{
        INVALID_POINTER=-1,
        TOUCH_MODE_UNKNOWN = -1,
        TOUCH_MODE_ON = 0,
        TOUCH_MODE_OFF = 1,
    };
    int mLastTouchMode = -1;
    int mTranscriptMode;
    int mCacheColorHint;
    int mDirection;
    int mFirstPositionDistanceGuess;
    int mLastPositionDistanceGuess;
    bool mSmoothScrollbarEnabled;
    bool mTextFilterEnabled;
    bool mPopupHidden;
    bool mFiltered;
    int mScrollOffset[2] ;
    int mScrollConsumed[2];
    class FastScroller* mFastScroll;
    std::vector<int>mSelectorState;
    int mLastScrollState;
    bool mIsChildViewEnabled;
    bool mForceTranscriptScroll;
    CheckForLongPress mPendingCheckForLongPress;
    CheckForTap mPendingCheckForTap;
    CheckForKeyLongPress mPendingCheckForKeyLongPress;
    AbsListView::PerformClick mPerformClick;
    FlingRunnable mFlingRunnable;
    Runnable mTouchModeReset;
    Runnable mClearScrollingCache;
    bool mHasPerformedLongPress;
    bool mScrollProfilingStarted = false;
    bool mFlingProfilingStarted =false;
    PopupWindow*mPopup;
    class EditText* mTextFilter;
    OnScrollListener mOnScrollListener;
    void initAbsListView(const AttributeSet&atts);
    void useDefaultSelector();
    std::vector<int>getDrawableStateForSelector();
    void setItemViewLayoutParams(View* child, int position);
    void initOrResetVelocityTracker();
    void initVelocityTrackerIfNotExists();
    void recycleVelocityTracker();
    bool canScrollUp()const;
    bool canScrollDown()const;
    void drawSelector(Canvas&canvas);
    void updateOnScreenCheckedViews();
    void onTouchUp(MotionEvent& ev);
    void onTouchDown(MotionEvent& ev);
    void onTouchMove(MotionEvent&,MotionEvent&);
    bool shouldDisplayEdgeEffects()const;
    void onTouchCancel();
    void onSecondaryPointerUp(MotionEvent&);
    bool contentFits();
    void positionSelector(int position, View* sel, bool manageHotspot, float x, float y);
    bool startScrollIfNeeded(int x, int y, MotionEvent* vtev);
    void scrollIfNeeded(int x, int y, MotionEvent* vtev);
    void setFastScrollerEnabledUiThread(bool enabled);
    void setFastScrollerAlwaysVisibleUiThread(bool alwaysShow);
    int  releaseGlow(int deltaY, int x);
    bool isGlowActive()const;
    void invalidateTopGlow();
    void invalidateBottomGlow();
    void finishGlows();
    void createScrollingCache();
    void clearScrollingCache();
    void dismissPopup();
    void showPopup();
    void positionPopup();
    bool acceptFilter()const;
    void createTextFilter(bool animateEntrance);
    EditText* getTextFilterInput();
protected:
    int mChoiceMode;
    int mCheckedItemCount;
    int mTouchSlop;
    float mDensityScale;
    int mSelectedTop;
    int mStackFromBottom;
    bool mScrollingCacheEnabled;
    bool mFastScrollEnabled;
    bool mFastScrollAlwaysVisible;
    std::string mFastScrollStyle;
    bool mGlobalLayoutListenerAddedFilter;
    int mSelectorPosition;
    int mResurrectToPosition;
    int mMinimumVelocity;
    int mMaximumVelocity;
    float mVelocityScale = 1.0f;
    int mOverscrollDistance;
    int mOverflingDistance;
    View * mScrollUp ;
    View * mScrollDown;
    bool mCachingStarted;
    bool mCachingActive;
    Runnable mPositionScrollAfterLayout;
    EdgeEffect* mEdgeGlowTop;
    EdgeEffect* mEdgeGlowBottom;
    AbsPositionScroller* mPositionScroller;
    int mLastHandledItemCount;
    bool mDrawSelectorOnTop;
    bool mAdapterHasStableIds;
    int mSelectionLeftPadding;
    int mSelectionTopPadding ;
    int mSelectionRightPadding;
    int mSelectionBottomPadding;
    int mOverscrollMax;
    bool mIsScrap[2]; 
    bool mIsDetaching;
    Rect mSelectorRect;
    Rect mListPadding;/*The view's padding*/
    int mWidthMeasureSpec;
    float mVerticalScrollFactor;
    int mLayoutMode;
    RecycleBin*mRecycler;
    AdapterDataSetObserver*mDataSetObserver;
    SparseBooleanArray mCheckStates;
    LongSparseArray mCheckedIdStates;
    Drawable* mSelector;
    int mMotionPosition;
    int mMotionViewOriginalTop;
    int mMotionViewNewTop;
    int mMotionX,mMotionY;
    int mLastY;
    int mNestedYOffset;
    int mActivePointerId;
    int mMotionCorrection;
    int mTouchMode;
    VelocityTracker* mVelocityTracker;
    class ActionMode*mChoiceActionMode;
    MultiChoiceModeListener mMultiChoiceModeCallback;
    virtual void resetList();
    int computeVerticalScrollExtent()override;
    int computeVerticalScrollOffset()override;
    int computeVerticalScrollRange ()override;
    AbsPositionScroller*createPositionScroller();
    virtual bool trackMotionScroll(int deltaY, int incrementalDeltaY);
    void onMeasure(int widthMeasureSpec, int heightMeasureSpec)override;
    void updateScrollIndicators();
    void setScrollIndicatorViews(View* up, View* down);
    void internalSetPadding(int left, int top, int width, int height)override;
    void onSizeChanged(int w, int h, int oldw, int oldh)override;
    void handleBoundsChange();
    virtual bool touchModeDrawsInPressedState();
    virtual bool shouldShowSelector();
    void updateSelectorState();
    void drawableStateChanged()override;
    virtual void layoutChildren();
    bool shouldDrawSelector();
    void dispatchDraw(Canvas& canvas)override;
    void dispatchSetPressed(bool pressed)override;
    void onLayout(bool changed, int l, int t, int w, int h)override;
    void onCancelPendingInputEvents()override;
    void confirmCheckedPositionsById();
    void handleDataChanged()override;
    static int getDistance(const Rect& source,const Rect& dest, int direction);
    virtual View* obtainView(int position, bool*outMetadata);
    void positionSelector(int position, View* sel);
    void hideSelector();
    void setVisibleRangeHint(int start,int end);
    void setEdgeEffectColor(int color);
    void setBottomEdgeEffectColor( int color);
    void setTopEdgeEffectColor(int color);
    int  getTopEdgeEffectColor()const;
    int  getBottomEdgeEffectColor()const;
    int  reconcileSelectedPosition();
    void requestLayoutIfNecessary();
 
    bool resurrectSelection(); 
    bool resurrectSelectionIfNeeded();
    void onAttachedToWindow()override;
    void onDetachedFromWindow()override;
    void onWindowFocusChanged(bool)override;
    virtual void setSelectionInt(int position)=0; 
    virtual int getHeightForPosition(int position);
    virtual int getHeaderViewsCount()const;
    virtual int getFooterViewsCount()const;
    virtual int findMotionRow(int y)=0;
    virtual void fillGap(bool down)=0;
    int findClosestMotionRow(int y);
    
    void positionSelectorLikeTouch(int position, View* sel, float x, float y);
    void positionSelectorLikeFocus(int position, View* sel);
    void keyPressed();
    void invokeOnItemScrollListener();
    bool performStylusButtonPressAction(MotionEvent& ev);
    bool performLongPress(View* child,int longPressPosition,long longPressId);
    bool performLongPress(View* child,int longPressPosition,long longPressId,int x,int y);
    ViewGroup::LayoutParams*generateDefaultLayoutParams()const override;
    ViewGroup::LayoutParams*generateLayoutParams(const ViewGroup::LayoutParams* p)const override;
    bool checkLayoutParams(const ViewGroup::LayoutParams* p)const override;
    void onFocusChanged(bool gainFocus, int direction,Rect* previouslyFocusedRect)override;
    void onOverScrolled(int scrollX, int scrollY, bool clampedX, bool clampedY)override;
    void draw(Canvas&canvas)override;
public:
    AbsListView(int w,int h);
    AbsListView(Context*,const AttributeSet&atts);
    ~AbsListView();
    void setAdapter(Adapter*adapter)override;
    int getCheckedItemCount()const;
    int getCheckedItemPositions(SparseBooleanArray&array);
    bool isItemChecked(int position)const;
    int getCheckedItemIds(std::vector<long>&ids)const;
    //Returns the currently checked item. The result is only valid forCHOICE_MODE_SINGLE
    int getCheckedItemPosition()const;
    void setItemChecked(int position, bool value);
    int getChoiceMode()const;
    void setChoiceMode(int choiceMode);
    void setMultiChoiceModeListener(MultiChoiceModeListener listener);
    void setOnScrollListener(OnScrollListener);
    bool isScrollingCacheEnabled()const;
    void setScrollingCacheEnabled(bool enabled);

    void reportScrollStateChange(int newState);
    void setFastScrollEnabled(bool);
    bool isFastScrollEnabled()const;
    int  getVerticalScrollbarWidth()const override;
    View& setVerticalScrollbarPosition(int position)override;
    void setFastScrollStyle(const std::string& styleid);
    void setFastScrollAlwaysVisible(bool alwaysShow);
    bool isFastScrollAlwaysVisible()const;

    void clearChoices();
    void setDrawSelectorOnTop(bool onTop);
    Drawable*getSelector();
    void setSelector(Drawable*drawable);
    void setSelector(const std::string&resid);
    void getFocusedRect(Rect& r)override;

    void setScrollBarStyle(int style);
    void setSmoothScrollbarEnabled(bool);
    bool isSmoothScrollbarEnabled()const;
    void setStackFromBottom(bool stackFromBottom);
    bool isStackFromBottom()const;

    bool isTextFilterEnabled()const;
    void setTextFilterEnabled(bool);
    void setFilterText(const std::string& filterText);
    void clearTextFilter();
    bool hasTextFilter()const;
    const std::string getTextFilter()const;
    void beforeTextChanged(const std::string& s, int start, int count, int after);//override textwatcher
    void onTextChanged(const std::string& s, int start, int before, int count);//override textwatcher
    void onFilterComplete(int count)override;
    bool verifyDrawable(Drawable* dr)const override;
    void jumpDrawablesToCurrentState()override;
    void dispatchDrawableHotspotChanged(float x, float y)override;
    int pointToPosition(int x, int y);
    long pointToRowId(int x, int y);
    bool performItemClick(View& view, int position, long id)override;
    bool onKeyDown(int keyCode, KeyEvent& event)override;
    bool onKeyUp(int keyCode, KeyEvent& event)override;
    bool onInterceptTouchEvent(MotionEvent& ev)override;
    bool onTouchEvent(MotionEvent& ev)override;
    void onRtlPropertiesChanged(int layoutDirection)override;
    void onTouchModeChanged(bool isInTouchMode);//called by ViewTreeObserver
    bool onGenericMotionEvent(MotionEvent& event)override;
    void fling(int velocity);
    bool onStartNestedScroll(View* child, View* target, int nestedScrollAxes);
    void onNestedScrollAccepted(View* child, View* target, int axes);
    void onNestedScroll(View* target, int dxConsumed, int dyConsumed,int dxUnconsumed, int dyUnconsumed);
    bool onNestedFling(View* target, float velocityX, float velocityY, bool consumed);
    void requestDisallowInterceptTouchEvent(bool disallowIntercept)override;
    void addTouchables(std::vector<View*>& views)override;
    View* getSelectedView()override;
    ViewGroup::LayoutParams*generateLayoutParams(const AttributeSet& attrs)const override;
    void setSelectionFromTop(int position, int y);
    int getCacheColorHint()const;
    void setCacheColorHint(int color);
    int getListPaddingTop()const;
    int getListPaddingBottom()const;
    int getListPaddingLeft()const;
    int getListPaddingRight()const;
    void setTranscriptMode(int);
    int getTranscriptMode()const;

    void setFriction(float friction);
    void setVelocityScale(float scale);
    void smoothScrollToPosition(int position);
    void smoothScrollToPositionFromTop(int position, int offset, int duration);
    void smoothScrollToPositionFromTop(int position, int offset);
    void smoothScrollToPosition(int position, int boundPosition);
    void smoothScrollBy(int distance, int duration);
    void smoothScrollBy(int distance, int duration, bool linear,bool suppressEndFlingStateChangeCall);
    void smoothScrollByOffset(int position);
    void scrollListBy(int y);
    bool canScrollList(int direction);
    void reclaimViews(std::vector<View*>& views);
};

}//namespace
#endif
