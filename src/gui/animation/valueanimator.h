#pragma once
#include <animation/animator.h>
#include <animation/propertyvaluesholder.h>
#include <map>

namespace cdroid{

typedef std::function<float (float fraction, float startValue, float endValue)>TypeEvaluator; 

class ValueAnimator:public Animator{
public:
    static constexpr int RESTART = 1;
    static constexpr int REVERSE = 2;
    static constexpr int INFINITE= -1;
    typedef std::function<void(ValueAnimator&)>AnimatorUpdateListener;
private:
    static constexpr float sDurationScale = 1.0f;
    //static TimeInterpolator sDefaultInterpolator = new AccelerateDecelerateInterpolator();
    long mPauseTime;
    bool mResumed = false;
    bool mReversing;
    float mOverallFraction = .0f;
    float mCurrentFraction = .0f;
    long mLastFrameTime = -1;
    long mFirstFrameTime = -1;
    bool mRunning = false;
    bool mStarted = false;
    bool mStartListenersCalled = false;
    bool mAnimationEndRequested = false;
    long mDuration = 300;
    long mStartDelay = 0;
    int mRepeatCount = 0;
    int mRepeatMode = RESTART;
    bool mSelfPulse = true;
    bool mSuppressSelfPulseRequested = false;
    float mDurationScale = -1.f;
    TimeInterpolator mInterpolator = nullptr;
    std::vector<AnimatorUpdateListener> mUpdateListeners;
    std::map<const std::string,PropertyValuesHolder*>mValuesMap;
protected:
    long mStartTime = -1;
    bool mStartTimeCommitted;
    float mSeekFraction = -1;
    bool mInitialized = false;
private:
    float resolveDurationScale();
    long getScaledDuration();
    int getCurrentIteration(float fraction);
    float getCurrentIterationFraction(float fraction, bool inReverse);
    float clampFraction(float fraction);
    bool shouldPlayBackward(int iteration, bool inReverse);
    bool isPulsingInternal();
    void notifyStartListeners();
    void start(bool playBackwards);
    void startAnimation();
    void endAnimation();
    void addOneShotCommitCallback();
    void removeAnimationCallback();
    void addAnimationCallback(long delay);
protected:
    void animateValue(float fraction);
    void startWithoutPulsing(bool inReverse);
    bool animateBasedOnTime(long currentTime);
    void animateBasedOnPlayTime(long currentPlayTime, long lastPlayTime, bool inReverse);
    void skipToEndValue(bool inReverse);
    bool isInitialized();
    bool pulseAnimationFrame(long frameTime);
public:
    ValueAnimator();
    ~ValueAnimator();
    static ValueAnimator* ofInt(int,...);
    static ValueAnimator* ofArgb(int,...);
    static ValueAnimator* ofFloat(int,...);
    void setValues(int count,...);
    void setIntValues(const std::vector<int>&);
    void setFloatValues(const std::vector<float>&);
    void initAnimation();
    ValueAnimator& setDuration(long duration);
    long getDuration();
    long getTotalDuration();
    void setCurrentPlayTime(long playTime);
    void setCurrentFraction(float fraction);
    long getCurrentPlayTime();
    long getStartDelay();
    void setStartDelay(long startDelay);
    static long getFrameDelay();
    static void setFrameDelay(long frameDelay);
    float getAnimatedValue();
    float getAnimatedValue(const std::string&propertyName);
    void setRepeatCount(int value);
    int getRepeatCount()const;
    void setRepeatMode(int value);
    int getRepeatMode()const;
    void addUpdateListener(AnimatorUpdateListener listener);
    void removeUpdateListener(AnimatorUpdateListener listener);
    void removeAllUpdateListeners();
    void setInterpolator(TimeInterpolator value);
    TimeInterpolator getInterpolator();
    void setEvaluator(TypeEvaluator value);
    void start()override;
    void cancel()override;
    void end()override;
    void resume()override;
    void pause()override;
    bool isRunning()override;
    bool isStarted()override;
    void reverse()override;
    bool canReverse()override;

    void commitAnimationFrame(long frameTime);
    bool doAnimationFrame(long frameTime);
    float getAnimatedFraction();
};

}//endof namespace
