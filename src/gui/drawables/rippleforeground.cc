#include <drawables/rippleforeground.h>
#include <core/systemclock.h>
#include <cdlog.h>

namespace cdroid{

RippleForeground::RippleForeground(RippleDrawable* owner,const Rect& bounds, float startingX, float startingY,
            bool forceSoftware):RippleComponent(owner, bounds){

    mForceSoftware = forceSoftware;
    mStartingX = startingX;
    mStartingY = startingY;
    mUsingProperties = false;
    mHasFinishedExit = false;
    mStartRadius  = .0f;
    mTargetRadius = .0f;
    mOpacity = .0f;
    mTweenRadius = .0f;
    mClampedStartingX = .0f;
    mClampedStartingY = .0f;
    // Take 60% of the maximum of the width and height, then divided half to get the radius.
    mStartRadius = std::max(bounds.width, bounds.height) * 0.3f;
    clampStartingPosition();
    mAnimationListener.onAnimationEnd=[this](Animator&anim,bool isReverse){
         mHasFinishedExit = true;
         pruneSwFinished();
    };
}

RippleForeground::~RippleForeground(){
    mUsingProperties =true;
    end();
}

void RippleForeground::onTargetRadiusChanged(float targetRadius){
    clampStartingPosition();
    LOGD("radius=%.2f anims=%d",targetRadius,mRunningSwAnimators.size());
    for (auto animator:mRunningSwAnimators) {
        animator->removeListener(mAnimationListener);
        animator->end();
        delete animator;
    }
    mRunningSwAnimators.clear();
    invalidateSelf();//switchToUiThreadAnimation();
}

void RippleForeground::drawSoftware(Canvas& c,float origAlpha) {
    const int alpha = (int) (origAlpha * mOpacity + 0.5f);
    const float radius = getCurrentRadius();
    if (alpha > 0 && radius > 0) {
        const float x = getCurrentX();
        const float y = getCurrentY();
        c.arc(x,y, radius,0,M_PI*2.);
        c.fill();
    }
}

void RippleForeground::pruneSwFinished() {
    if( mRunningSwAnimators.size()==0)return;
    for (int i=mRunningSwAnimators.size()-1;i>=0;i--){
        Animator*anim=mRunningSwAnimators[i];
        if (!anim->isRunning()) {
            mRunningSwAnimators.erase(mRunningSwAnimators.begin()+i);
            delete anim;
        }
    }
}

void RippleForeground::getBounds(Rect& bounds) {
    const int outerX = (int) mTargetX;
    const int outerY = (int) mTargetY;
    const int r = (int) mTargetRadius + 1;
    bounds.set(outerX - r, outerY - r, r+r,r + r);
}

void RippleForeground::move(float x, float y) {
    mStartingX = x;
    mStartingY = y;
    clampStartingPosition();
}

bool RippleForeground::hasFinishedExit()const{
    return mHasFinishedExit;
}

long RippleForeground::computeFadeOutDelay() {
    long timeSinceEnter = AnimationUtils::currentAnimationTimeMillis() - mEnterStartedAtMillis;
    if (timeSinceEnter > 0 && timeSinceEnter < OPACITY_HOLD_DURATION) {
        return OPACITY_HOLD_DURATION - timeSinceEnter;
    }
    return 0;
}

float RippleForeground::getOpacity()const{
    return mOpacity;
}

void RippleForeground::startSoftwareEnter() {
    for (auto anim:mRunningSwAnimators) {
        anim->cancel();
        delete anim;
    }
    mRunningSwAnimators.clear();
    ValueAnimator* tweenRadius = ValueAnimator::ofFloat({.0f,1.f});//this, TWEEN_RADIUS, 1);
    tweenRadius->setDuration(RIPPLE_ENTER_DURATION);
    tweenRadius->setInterpolator(DecelerateInterpolator::gDecelerateInterpolator.get());//DECELERATE_INTERPOLATOR);
    tweenRadius->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        LOGV("mTweenRadius=%f [%f,%f,%f] opacity=%f",getCurrentRadius(),mStartRadius,mTargetRadius,mOpacity);
        mTweenRadius = GET_VARIANT(anim.getAnimatedValue(),float);
        onAnimationPropertyChanged();
    }));
    tweenRadius->start();
    mRunningSwAnimators.push_back(tweenRadius);

    ValueAnimator* tweenOrigin = ValueAnimator::ofFloat({.0f,1.f});//this, TWEEN_ORIGIN, 1);
    tweenOrigin->setDuration(RIPPLE_ORIGIN_DURATION);
    tweenOrigin->setInterpolator(DecelerateInterpolator::gDecelerateInterpolator.get());//DECELERATE_INTERPOLATOR);
    tweenOrigin->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        mTweenX=mTweenY=GET_VARIANT(anim.getAnimatedValue(),float);
        onAnimationPropertyChanged();
    }));
    tweenOrigin->start();
    mRunningSwAnimators.push_back(tweenOrigin);
	
    ValueAnimator* opacity = ValueAnimator::ofFloat({.0f,1.f});//this, OPACITY, 1);
    opacity->setDuration(OPACITY_ENTER_DURATION);
    opacity->setInterpolator(LinearInterpolator::gLinearInterpolator.get());//LINEAR_INTERPOLATOR);
    opacity->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        mOpacity=GET_VARIANT(anim.getAnimatedValue(),float);
        onAnimationPropertyChanged();
    }));
    opacity->start();
    mRunningSwAnimators.push_back(opacity);
}

void RippleForeground::startSoftwareExit() {
    ValueAnimator* opacity = ValueAnimator::ofFloat({.0f,1.f});
    opacity->setDuration(OPACITY_EXIT_DURATION);
    opacity->setInterpolator(LinearInterpolator::gLinearInterpolator.get());//LINEAR_INTERPOLATOR);
    opacity->addListener(mAnimationListener);
    opacity->setStartDelay(computeFadeOutDelay());
    opacity->addUpdateListener(ValueAnimator::AnimatorUpdateListener([this](ValueAnimator&anim){
        mOpacity=GET_VARIANT(anim.getAnimatedValue(),float);
    }));

    opacity->start();
    mRunningSwAnimators.push_back(opacity);
}

void RippleForeground::enter(){
    mEnterStartedAtMillis = AnimationUtils::currentAnimationTimeMillis();
    startSoftwareEnter();
}

void RippleForeground::exit(){
    startSoftwareExit();
}

float RippleForeground::getCurrentX() {
    return lerp(mClampedStartingX - mBounds.centerX(), mTargetX, mTweenX);
}

float RippleForeground::getCurrentY() {
    return lerp(mClampedStartingY - mBounds.centerY(), mTargetY, mTweenY);
}

float RippleForeground::getCurrentRadius() {
    return lerp(mStartRadius, mTargetRadius, mTweenRadius);
}

void RippleForeground::end(){
    for (auto anim:mRunningSwAnimators) {
        anim->end();
        delete anim;
    }
    mRunningSwAnimators.clear();
}

void RippleForeground::onAnimationPropertyChanged() {
    if (!mUsingProperties) {
        invalidateSelf();
    }
}

void RippleForeground::draw(Canvas&canvas,float alpha){
    pruneSwFinished();
    drawSoftware(canvas,alpha);
}

void RippleForeground::clampStartingPosition(){
    float cX = mBounds.centerX();
    float cY = mBounds.centerY();
    float dX = mStartingX - cX;
    float dY = mStartingY - cY;
    float r = mTargetRadius - mStartRadius;
    if (dX * dX + dY * dY > r * r) {
        // Point is outside the circle, clamp to the perimeter.
        double angle = std::atan2(dY, dX);
        mClampedStartingX = cX + (float) (std::cos(angle) * r);
        mClampedStartingY = cY + (float) (std::sin(angle) * r);
    } else {
        mClampedStartingX = mStartingX;
        mClampedStartingY = mStartingY;
    }
}

}
