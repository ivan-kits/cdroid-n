#include <widget/forwardinglistener.h>
#include <widget/dropdownlistview.h>
#include <cdlog.h>
namespace cdroid{

ForwardingListener::ForwardingListener(View* src){
    mSrc = src;
    //src->setLongClickable(true);
    //src->addOnAttachStateChangeListener(this);

    mScaledTouchSlop = ViewConfiguration::get(src->getContext()).getScaledTouchSlop();
    mTapTimeout = ViewConfiguration::getTapTimeout();
    mDisallowIntercept =[this](){
        ViewGroup* parent = mSrc->getParent();
        if (parent != nullptr) {
            parent->requestDisallowInterceptTouchEvent(true);
        }
    };
    mTriggerLongPress = [this](){
        onLongPress();
    };
    // Use a medium-press timeout. Halfway between tap and long-press.
    mLongPressTimeout = (mTapTimeout + ViewConfiguration::getLongPressTimeout()) / 2;
}

bool ForwardingListener::onTouch(View* v, MotionEvent& event){
    bool wasForwarding = mForwarding;
    bool forwarding;
    if (wasForwarding) {
        forwarding = onTouchForwarded(event) || !onForwardingStopped();
    } else {
        forwarding = onTouchObserved(event) && onForwardingStarted();

        if (forwarding) {
            // Make sure we cancel any ongoing source event stream.
            long now = SystemClock::uptimeMillis();
            MotionEvent* e = MotionEvent::obtain(now, now, MotionEvent::ACTION_CANCEL,0.0f, 0.0f, 0);
            mSrc->onTouchEvent(*e);
            e->recycle();
        }
    }
    mForwarding = forwarding;
    return forwarding || wasForwarding;
}

void ForwardingListener::onViewAttachedToWindow(View* v){
}

void ForwardingListener::onViewDetachedFromWindow(View*v){
    mForwarding = false;
    mActivePointerId = MotionEvent::INVALID_POINTER_ID;

    if (mDisallowIntercept != nullptr) {
        mSrc->removeCallbacks(mDisallowIntercept);
    }
}

bool ForwardingListener::onForwardingStarted() {
    ShowableListMenu popup = getPopup();
    if (popup.isShowing && !popup.isShowing()) {
        popup.show();
    }
    return true;
}

bool ForwardingListener::onForwardingStopped() {
    ShowableListMenu popup = getPopup();
    if (popup.isShowing && popup.isShowing()) {
        popup.dismiss();
    }
    return true;
}

bool ForwardingListener::onTouchObserved(MotionEvent& srcEvent){
    View* src = mSrc;
    if (!src->isEnabled()) {
        return false;
    }
    int activePointerIndex = -1;
    int actionMasked = srcEvent.getActionMasked();
    switch (actionMasked) {
    case MotionEvent::ACTION_DOWN:
         mActivePointerId = srcEvent.getPointerId(0);

         src->postDelayed(mDisallowIntercept, mTapTimeout);
         src->postDelayed(mTriggerLongPress, mLongPressTimeout);
         break;
     case MotionEvent::ACTION_MOVE:
         activePointerIndex = srcEvent.findPointerIndex(mActivePointerId);
         if (activePointerIndex >= 0) {
             float x = srcEvent.getX(activePointerIndex);
             float y = srcEvent.getY(activePointerIndex);

             // Has the pointer moved outside of the view?
             if (!src->pointInView(x, y, mScaledTouchSlop)) {
                 clearCallbacks();

                 // Don't let the parent intercept our events.
                 src->getParent()->requestDisallowInterceptTouchEvent(true);
                 return true;
             }
         }
         break;
     case MotionEvent::ACTION_CANCEL:
     case MotionEvent::ACTION_UP:
         clearCallbacks();
         break;
     }
     return false;
}

void ForwardingListener::clearCallbacks() {
    if (mTriggerLongPress) {
        mSrc->removeCallbacks(mTriggerLongPress);
    }

    if (mDisallowIntercept) {
        mSrc->removeCallbacks(mDisallowIntercept);
    }
}

void ForwardingListener::onLongPress() {
    clearCallbacks();
    LOGD("%p",this);
    if (!mSrc->isEnabled() || mSrc->isLongClickable()) {
        // Ignore long-press if the view is disabled or has its own
        // handler.
        return;
    }

    if (!onForwardingStarted()) {
        return;
    }

     // Don't let the parent intercept our events.
    mSrc->getParent()->requestDisallowInterceptTouchEvent(true);

    // Make sure we cancel any ongoing source event stream.
    long now = SystemClock::uptimeMillis();
    MotionEvent* e = MotionEvent::obtain(now, now, MotionEvent::ACTION_CANCEL, 0, 0, 0);
    mSrc->onTouchEvent(*e);
    e->recycle();
    mForwarding = true;
}

bool ForwardingListener::onTouchForwarded(MotionEvent& srcEvent){
    ShowableListMenu popup = getPopup();
    if (popup.isShowing == nullptr || !popup.isShowing()) {
        return false;
    }

    DropDownListView* dst = (DropDownListView*) popup.getListView();
    if (dst == nullptr || !dst->isShown()) {
        return false;
    }

    // Convert event to destination-local coordinates.
    MotionEvent* dstEvent = MotionEvent::obtainNoHistory(srcEvent);
    mSrc->toGlobalMotionEvent(*dstEvent);
    dst->toLocalMotionEvent(*dstEvent);

    // Forward converted event to destination view, then recycle it.
    bool handled = dst->onForwardedEvent(*dstEvent, mActivePointerId);
    dstEvent->recycle();

     // Always cancel forwarding when the touch stream ends.
    int action = srcEvent.getActionMasked();
    bool keepForwarding = action != MotionEvent::ACTION_UP
             && action != MotionEvent::ACTION_CANCEL;
    return handled && keepForwarding;
}

}
