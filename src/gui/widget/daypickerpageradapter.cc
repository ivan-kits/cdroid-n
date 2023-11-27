#include <widget/daypickerpageradapter.h>
namespace cdroid{

DayPickerPagerAdapter::ViewHolder::ViewHolder(int position, View* container, SimpleMonthView* calendar) {
    this->position = position;
    this->container = container;
    this->calendar = calendar;
}

DayPickerPagerAdapter::DayPickerPagerAdapter(Context* context,const std::string&layoutResId,int calendarViewId){
    mInflater = LayoutInflater::from(context);
    mLayoutResId = layoutResId;
    mCalendarViewId = calendarViewId;

    mOnDayClickListener=[this](SimpleMonthView& view, Calendar& day){
        setSelectedDay(&day);
        if (mOnDaySelectedListener) {
            mOnDaySelectedListener(*this, day);
        }
    };
    //final TypedArray ta = context.obtainStyledAttributes(new int[] {
    //            com.android.internal.R.attr.colorControlHighlight});
    //mDayHighlightColor = ta.getColorStateList(0);
    //ta.recycle();
}

DayPickerPagerAdapter::~DayPickerPagerAdapter(){
    delete mCalendarTextColor;
    delete mDaySelectorColor;
    delete mDayHighlightColor;
}

void DayPickerPagerAdapter::setRange(Calendar& min,Calendar& max) {
    mMinDate.setTimeInMillis(min.getTimeInMillis());
    mMaxDate.setTimeInMillis(max.getTimeInMillis());

    int diffYear = mMaxDate.get(Calendar::YEAR) - mMinDate.get(Calendar::YEAR);
    int diffMonth = mMaxDate.get(Calendar::MONTH) - mMinDate.get(Calendar::MONTH);
    mCount = diffMonth + MONTHS_IN_YEAR * diffYear + 1;

    // Positions are now invalid, clear everything and start over.
    notifyDataSetChanged();
}

void DayPickerPagerAdapter::setFirstDayOfWeek(int weekStart) {
    mFirstDayOfWeek = weekStart;

    // Update displayed views.
    int count = mItems.size();
    for (int i = 0; i < count; i++) {
        SimpleMonthView* monthView = mItems.get(i)->calendar;
        monthView->setFirstDayOfWeek(weekStart);
    }
}

int DayPickerPagerAdapter::getFirstDayOfWeek()const{
    return mFirstDayOfWeek;
}

bool DayPickerPagerAdapter::getBoundsForDate(Calendar& day, Rect& outBounds) {
    int position = getPositionForDay(&day);
    ViewHolder* monthView = mItems.get(position, nullptr);
    if (monthView == nullptr) {
        return false;
    } else {
        int dayOfMonth = day.get(Calendar::DAY_OF_MONTH);
        return monthView->calendar->getBoundsForDay(dayOfMonth, outBounds);
    }
}

void DayPickerPagerAdapter::setSelectedDay(Calendar* day) {
    int oldPosition = getPositionForDay(mSelectedDay);
    int newPosition = getPositionForDay(day);

    // Clear the old position if necessary.
    if (oldPosition != newPosition && oldPosition >= 0) {
        ViewHolder* oldMonthView = mItems.get(oldPosition, nullptr);
        if (oldMonthView != nullptr) {
            oldMonthView->calendar->setSelectedDay(-1);
        }
    }

    // Set the new position.
    if (newPosition >= 0) {
        ViewHolder* newMonthView = mItems.get(newPosition, nullptr);
        if (newMonthView != nullptr) {
            int dayOfMonth = day->get(Calendar::DAY_OF_MONTH);
            newMonthView->calendar->setSelectedDay(dayOfMonth);
        }
    }
    mSelectedDay = day;
}

void DayPickerPagerAdapter::setOnDaySelectedListener(OnDaySelectedListener listener) {
    mOnDaySelectedListener = listener;
}

void DayPickerPagerAdapter::setCalendarTextColor(const ColorStateList* calendarTextColor) {
    if(calendarTextColor==nullptr){
        delete mCalendarTextColor;
        mCalendarTextColor = nullptr;
    }else{
        if(mCalendarTextColor) *mCalendarTextColor=*calendarTextColor;
        else mCalendarTextColor = new ColorStateList(*calendarTextColor);
    }
    notifyDataSetChanged();
}

void DayPickerPagerAdapter::setDaySelectorColor(const ColorStateList* selectorColor) {
    if(selectorColor==nullptr){
        delete mDaySelectorColor;
        mDaySelectorColor = nullptr;
    }else{
        if(mDaySelectorColor) *mDaySelectorColor=*selectorColor;
        else mDaySelectorColor = new ColorStateList(*selectorColor);
    }
    notifyDataSetChanged();
}

void DayPickerPagerAdapter::setMonthTextAppearance(const std::string& resId) {
    mMonthTextAppearance = resId;
    notifyDataSetChanged();
}

void DayPickerPagerAdapter::setDayOfWeekTextAppearance(const std::string& resId) {
    mDayOfWeekTextAppearance = resId;
    notifyDataSetChanged();
}

std::string DayPickerPagerAdapter::getDayOfWeekTextAppearance() {
    return mDayOfWeekTextAppearance;
}

void DayPickerPagerAdapter::setDayTextAppearance(const std::string& resId) {
    mDayTextAppearance = resId;
    notifyDataSetChanged();
}

std::string DayPickerPagerAdapter::getDayTextAppearance() {
    return mDayTextAppearance;
}

int DayPickerPagerAdapter::getCount() {
    return mCount;
}

bool DayPickerPagerAdapter::isViewFromObject(View* view, void*object) {
    ViewHolder* holder = (ViewHolder*) object;
    return view == holder->container;
}

int DayPickerPagerAdapter::getMonthForPosition(int position) {
    return (position + mMinDate.get(Calendar::MONTH)) % MONTHS_IN_YEAR;
}

int DayPickerPagerAdapter::getYearForPosition(int position) {
    int yearOffset = (position + mMinDate.get(Calendar::MONTH)) / MONTHS_IN_YEAR;
    return yearOffset + mMinDate.get(Calendar::YEAR);
}

int DayPickerPagerAdapter::getPositionForDay(Calendar* day) {
    if (day == nullptr) {
        return -1;
    }

    int yearOffset = day->get(Calendar::YEAR) - mMinDate.get(Calendar::YEAR);
    int monthOffset = day->get(Calendar::MONTH) - mMinDate.get(Calendar::MONTH);
    int position = yearOffset * MONTHS_IN_YEAR + monthOffset;
    return position;
}

void* DayPickerPagerAdapter::instantiateItem(ViewGroup* container, int position) {
    View* itemView = mInflater->inflate(mLayoutResId, container, false);

    SimpleMonthView* v = (SimpleMonthView*)itemView->findViewById(mCalendarViewId);
    v->setOnDayClickListener(mOnDayClickListener);
    v->setMonthTextAppearance(mMonthTextAppearance);
    v->setDayOfWeekTextAppearance(mDayOfWeekTextAppearance);
    v->setDayTextAppearance(mDayTextAppearance);

    if (mDaySelectorColor != nullptr) {
        v->setDaySelectorColor(mDaySelectorColor);
    }

    if (mDayHighlightColor != nullptr) {
        v->setDayHighlightColor(mDayHighlightColor);
    }

    if (mCalendarTextColor != nullptr) {
        v->setMonthTextColor(mCalendarTextColor);
        v->setDayOfWeekTextColor(mCalendarTextColor);
        v->setDayTextColor(mCalendarTextColor);
    }

    int month = getMonthForPosition(position);
    int year = getYearForPosition(position);

    int selectedDay;
    if (mSelectedDay != nullptr && mSelectedDay->get(Calendar::MONTH) == month) {
        selectedDay = mSelectedDay->get(Calendar::DAY_OF_MONTH);
    } else {
        selectedDay = -1;
    }

    int enabledDayRangeStart;
    if (mMinDate.get(Calendar::MONTH) == month && mMinDate.get(Calendar::YEAR) == year) {
        enabledDayRangeStart = mMinDate.get(Calendar::DAY_OF_MONTH);
    } else {
        enabledDayRangeStart = 1;
    }

    int enabledDayRangeEnd;
    if (mMaxDate.get(Calendar::MONTH) == month && mMaxDate.get(Calendar::YEAR) == year) {
        enabledDayRangeEnd = mMaxDate.get(Calendar::DAY_OF_MONTH);
    } else {
        enabledDayRangeEnd = 31;
    }

    v->setMonthParams(selectedDay, month, year, mFirstDayOfWeek,
            enabledDayRangeStart, enabledDayRangeEnd);

    ViewHolder* holder = new ViewHolder(position, itemView, v);
    mItems.put(position, holder);

    container->addView(itemView);

    return holder;
}

void DayPickerPagerAdapter::destroyItem(ViewGroup* container, int position,void* object) {
    ViewHolder* holder = (ViewHolder*) object;
    container->removeView(holder->container);
    mItems.remove(position);
}

int DayPickerPagerAdapter::getItemPosition(void* object) {
    ViewHolder* holder = (ViewHolder*) object;
    return holder->position;
}

std::string DayPickerPagerAdapter::getPageTitle(int position) {
    SimpleMonthView* v = mItems.get(position)->calendar;
    if (v != nullptr) {
        return v->getMonthYearLabel();
    }
    return nullptr;
}

SimpleMonthView* DayPickerPagerAdapter::getView(void* object) {
    if (object == nullptr) {
        return nullptr;
    }
    ViewHolder* holder = (ViewHolder*) object;
    return holder->calendar;
}

}//endof namespace
