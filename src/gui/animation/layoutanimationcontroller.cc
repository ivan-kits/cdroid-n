#include <stdlib.h>
#include <widget/viewgroup.h>
#include <animation/layoutanimationcontroller.h>
namespace cdroid{

LayoutAnimationController::LayoutAnimationController(Context* context, const AttributeSet& attrs){
}

LayoutAnimationController::LayoutAnimationController(Animation* animation,float delay){
    mDelay = delay;
    setAnimation(animation);
}

LayoutAnimationController::LayoutAnimationController(Animation* animation)
 :LayoutAnimationController(animation,.5f){
}

int LayoutAnimationController::getOrder()const{
    return mOrder;
}

void LayoutAnimationController::setOrder(int order){
    mOrder = order;
}

void LayoutAnimationController::setAnimation(Context* context,const std::string&resourceID){

}

void LayoutAnimationController::setAnimation(Animation* animation){
    mAnimation = animation;
    mAnimation->setFillBefore(true);
}

Animation* LayoutAnimationController::getAnimation(){
    return mAnimation;
}

void LayoutAnimationController::setInterpolator(Context* context,const std::string&resourceID){
}

void LayoutAnimationController::setInterpolator(Interpolator* interpolator){
    mInterpolator = interpolator;
}

Interpolator* LayoutAnimationController::getInterpolator(){
    return mInterpolator;
}

float LayoutAnimationController::getDelay()const{
    return mDelay;
}

void LayoutAnimationController::setDelay(float delay){
    mDelay= delay;
}

bool LayoutAnimationController::willOverlap(){
    return mDelay < 1.0f;
}

void LayoutAnimationController::start(){
    mDuration = mAnimation->getDuration();
    mMaxDelay = LONG_MIN;//Long.MIN_VALUE;
    mAnimation->setStartTime(-1);
}

Animation* LayoutAnimationController::getAnimationForView(View* view){
    long delay = getDelayForView(view) + mAnimation->getStartOffset();
    mMaxDelay = std::max(mMaxDelay, delay);

    Animation* animation = mAnimation->clone();
    animation->setStartOffset(delay);
    return animation;
}

bool LayoutAnimationController::isDone()const{
    return SystemClock::uptimeMillis()>//AnimationUtils.currentAnimationTimeMillis() >
                mAnimation->getStartTime() + mMaxDelay + mDuration;
}

long LayoutAnimationController::getDelayForView(View* view){
    ViewGroup::LayoutParams* lp = view->getLayoutParams();
    AnimationParameters* params = lp->layoutAnimationParameters;

    if (params == nullptr) return 0;

    float delay = mDelay * mAnimation->getDuration();
    long viewDelay = (long) (getTransformedIndex(params) * delay);
    float totalDelay = delay * params->count;

    if (mInterpolator == nullptr) {
        mInterpolator = new LinearInterpolator();
    }

    float normalizedDelay = viewDelay / totalDelay;
    normalizedDelay = mInterpolator->getInterpolation(normalizedDelay);

    return (long) (normalizedDelay * totalDelay);
}

int LayoutAnimationController::getTransformedIndex(const AnimationParameters* params){
    switch (getOrder()) {
    case ORDER_REVERSE:
        return params->count - 1 - params->index;
    case ORDER_RANDOM:
        return (int) (params->count * drand48());
    case ORDER_NORMAL:
    default: return params->index;
    }
}

}
