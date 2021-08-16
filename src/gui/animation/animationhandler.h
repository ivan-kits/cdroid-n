#ifndef __ANIMATION_HANDLER__
#define __ANIMATION_HANDLER__
#include <core/choreographer.h>
#include <core/looper.h>
#include <map>
namespace cdroid{
class ObjectAnimator;
class AnimationHandler:EventHandler{
public:
    struct AnimationFrameCallback{
        virtual bool doAnimationFrame(long)=0;
        virtual void commitAnimationFrame(long)=0;
    };
    class AnimationFrameCallbackProvider {
    public:
        virtual void postFrameCallback(const Choreographer::FrameCallback& callback)=0;
        virtual void postCommitCallback(const Runnable& runnable)=0;
        virtual long getFrameTime()=0;
        virtual long getFrameDelay()=0;
        virtual void setFrameDelay(long delay)=0;
    };
private:
    class MyFrameCallbackProvider :public AnimationFrameCallbackProvider {
    public:
        void postFrameCallback(const Choreographer::FrameCallback& callback)override;
        void postCommitCallback(const Runnable& runnable)override;
        long getFrameTime()override;
        long getFrameDelay()override;
        void setFrameDelay(long delay)override;
    };
    bool mListDirty;
    AnimationFrameCallbackProvider* mProvider;
    Choreographer::FrameCallback mFrameCallback;
    static AnimationHandler*mInst;
    std::list<AnimationFrameCallback*> mAnimationCallbacks;
    std::list<AnimationFrameCallback*> mCommitCallbacks;
    std::map<AnimationFrameCallback*,long>mDelayedCallbackStartTime;
private:
    AnimationFrameCallbackProvider* getProvider();
    AnimationHandler();
    void doFrame(long);
    void doAnimationFrame(long frameTime);
    bool isCallbackDue(AnimationFrameCallback* callback, long currentTime);
    void commitAnimationFrame(AnimationFrameCallback* callback, long frameTime);
    void cleanUpList();
    int getCallbackSize()const;
protected:
    int checkEvents()override;
    int handleEvents()override;
public:
    static AnimationHandler& getInstance();
    void setProvider(const AnimationFrameCallbackProvider* provider);
    void addAnimationFrameCallback(const AnimationFrameCallback* callback, long delay);
    void addOneShotCommitCallback(const AnimationFrameCallback* callback);
    void removeCallback(const AnimationFrameCallback* callback);
    static int getAnimationCount();
    static void setFrameDelay(long delay);
    static long getFrameDelay();
    void autoCancelBasedOn(ObjectAnimator* objectAnimator);
};


}//endof namespace
#endif
