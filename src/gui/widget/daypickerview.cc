#include <widget/daypickerview.h>
#include <widget/simplemonthview.h>

namespace cdroid{

void DayPickerView::updateButtonVisibility(int position) {
    bool hasPrev = position > 0;
    bool hasNext = position < (mAdapter->getCount() - 1);
    mPrevButton->setVisibility(hasPrev ? View::VISIBLE : View::INVISIBLE);
    mNextButton->setVisibility(hasNext ? View::VISIBLE : View::INVISIBLE);
}

void DayPickerView::onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
    ViewPager* viewPager = mViewPager;
    measureChild(viewPager, widthMeasureSpec, heightMeasureSpec);

    int measuredWidthAndState = viewPager->getMeasuredWidthAndState();
    int measuredHeightAndState = viewPager->getMeasuredHeightAndState();
    setMeasuredDimension(measuredWidthAndState, measuredHeightAndState);

    int pagerWidth = viewPager->getMeasuredWidth();
    int pagerHeight = viewPager->getMeasuredHeight();
    int buttonWidthSpec = MeasureSpec::makeMeasureSpec(pagerWidth, MeasureSpec::AT_MOST);
    int buttonHeightSpec = MeasureSpec::makeMeasureSpec(pagerHeight, MeasureSpec::AT_MOST);
    mPrevButton->measure(buttonWidthSpec, buttonHeightSpec);
    mNextButton->measure(buttonWidthSpec, buttonHeightSpec);
}

void DayPickerView::onLayout(bool changed, int left, int top, int width, int height) {
    ImageButton* leftButton;
    ImageButton* rightButton;
    if (isLayoutRtl()) {
        leftButton = mNextButton;
        rightButton = mPrevButton;
    } else {
        leftButton = mPrevButton;
        rightButton = mNextButton;
    }

    mViewPager->layout(0, 0, width, height);

    SimpleMonthView* monthView = (SimpleMonthView*) mViewPager->getChildAt(0);
    int monthHeight = monthView->getMonthHeight();
    int cellWidth = monthView->getCellWidth();

    // Vertically center the previous/next buttons within the month
    // header, horizontally center within the day cell.
    int leftDW = leftButton->getMeasuredWidth();
    int leftDH = leftButton->getMeasuredHeight();
    int leftIconTop = monthView->getPaddingTop() + (monthHeight - leftDH) / 2;
    int leftIconLeft = monthView->getPaddingLeft() + (cellWidth - leftDW) / 2;
        leftButton->layout(leftIconLeft, leftIconTop, leftIconLeft + leftDW, leftIconTop + leftDH);

    int rightDW = rightButton->getMeasuredWidth();
    int rightDH = rightButton->getMeasuredHeight();
    int rightIconTop = monthView->getPaddingTop() + (monthHeight - rightDH) / 2;
    int rightIconRight = width - monthView->getPaddingRight() - (cellWidth - rightDW) / 2;
    rightButton->layout(rightIconRight - rightDW, rightIconTop,
                rightIconRight, rightIconTop + rightDH);
}

void DayPickerView::setDayOfWeekTextAppearance(const std::string& resId) {
    mAdapter->setDayOfWeekTextAppearance(resId);
}

const std::string DayPickerView::getDayOfWeekTextAppearance() {
    return mAdapter->getDayOfWeekTextAppearance();
}

void DayPickerView::setDayTextAppearance(const std::string& resId) {
    mAdapter->setDayTextAppearance(resId);
}

const std::string DayPickerView::getDayTextAppearance() {
    return mAdapter->getDayTextAppearance();
}

void DayPickerView::setDate(int64_t timeInMillis) {
    setDate(timeInMillis, false);
}

void DayPickerView::setDate(int64_t timeInMillis, bool animate) {
    setDate(timeInMillis, animate, true);
}

void DayPickerView::setDate(int64_t timeInMillis, bool animate, bool setSelected) {
    bool dateClamped = false;
    // Clamp the target day in milliseconds to the min or max if outside the range.
    if (timeInMillis < mMinDate.getTimeInMillis()) {
        timeInMillis = mMinDate.getTimeInMillis();
        dateClamped = true;
    } else if (timeInMillis > mMaxDate.getTimeInMillis()) {
        timeInMillis = mMaxDate.getTimeInMillis();
        dateClamped = true;
    }

    getTempCalendarForTime(timeInMillis);

    if (setSelected || dateClamped) {
        mSelectedDay.setTimeInMillis(timeInMillis);
    }

    int position = getPositionFromDay(timeInMillis);
    if (position != mViewPager->getCurrentItem()) {
        mViewPager->setCurrentItem(position, animate);
    }

    mAdapter->setSelectedDay(&mTempCalendar);
}

int64_t DayPickerView::getDate() {
    return mSelectedDay.getTimeInMillis();
}

bool DayPickerView::getBoundsForDate(int64_t timeInMillis,Rect& outBounds) {
    int position = getPositionFromDay(timeInMillis);
    if (position != mViewPager->getCurrentItem()) {
        return false;
    }

    mTempCalendar.setTimeInMillis(timeInMillis);
    return mAdapter->getBoundsForDate(mTempCalendar, outBounds);
}

void DayPickerView::setFirstDayOfWeek(int firstDayOfWeek) {
    mAdapter->setFirstDayOfWeek(firstDayOfWeek);
}

int DayPickerView::getFirstDayOfWeek() {
    return mAdapter->getFirstDayOfWeek();
}

void DayPickerView::setMinDate(int64_t timeInMillis) {
    mMinDate.setTimeInMillis(timeInMillis);
    onRangeChanged();
}

int64_t DayPickerView::getMinDate() {
    return mMinDate.getTimeInMillis();
}

void DayPickerView::setMaxDate(int64_t timeInMillis) {
    mMaxDate.setTimeInMillis(timeInMillis);
    onRangeChanged();
}

int64_t DayPickerView::getMaxDate() {
    return mMaxDate.getTimeInMillis();
}

/**
 * Handles changes to date range.
 */
void DayPickerView::onRangeChanged() {
    mAdapter->setRange(mMinDate, mMaxDate);

    // Changing the min/max date changes the selection position since we
    // don't really have stable IDs. Jumps immediately to the new position.
    setDate(mSelectedDay.getTimeInMillis(), false, false);

    updateButtonVisibility(mViewPager->getCurrentItem());
}

/**
 * Sets the listener to call when the user selects a day.
 *
 * @param listener The listener to call.
 */
void DayPickerView::setOnDaySelectedListener(OnDaySelectedListener listener) {
    mOnDaySelectedListener = listener;
}

int DayPickerView::getDiffMonths(Calendar& start, Calendar& end) {
    int diffYears = end.get(Calendar::YEAR) - start.get(Calendar::YEAR);
    return end.get(Calendar::MONTH) - start.get(Calendar::MONTH) + 12 * diffYears;
}

static int constrain(int amount, int low, int high) {//get the
    return amount < low ? low : (amount > high ? high : amount);
}

int DayPickerView::getPositionFromDay(int64_t timeInMillis) {
    Calendar ctmp=getTempCalendarForTime(timeInMillis);
    int diffMonthMax = getDiffMonths(mMinDate, mMaxDate);
    int diffMonth = getDiffMonths(mMinDate, ctmp);
    return constrain(diffMonth, 0, diffMonthMax);
}

Calendar DayPickerView::getTempCalendarForTime(int64_t timeInMillis) {
    mTempCalendar.setTimeInMillis(timeInMillis);
    return mTempCalendar;
}

/**
 * Gets the position of the view that is most prominently displayed within the list view.
 */
int DayPickerView::getMostVisiblePosition() {
    return mViewPager->getCurrentItem();
}

void DayPickerView::setPosition(int position) {
    mViewPager->setCurrentItem(position, false);
}

}//namespace
