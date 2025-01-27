#include <uieventsource.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <systemclock.h>
#include <list>


namespace cdroid{

UIEventSource::UIEventSource(View*v,std::function<void()>r){
    mAttachedView = dynamic_cast<ViewGroup*>(v);
    mLayoutRunner = r;
}

UIEventSource::~UIEventSource(){
}

int UIEventSource::checkEvents(){
    return hasDelayedRunners()||(mAttachedView&&mAttachedView->isDirty())
           ||mAttachedView->isLayoutRequested()
	   //||mAttachInfo->mViewRequestingLayout
	   ||GraphDevice::getInstance().needCompose();
}

int UIEventSource::handleEvents(){
    GraphDevice::getInstance().lock();
    if (mAttachedView && mAttachedView->isAttachedToWindow()){
        if(mAttachedView->isLayoutRequested())
            mLayoutRunner();
        if(mAttachedView->isDirty() && mAttachedView->getVisibility()==View::VISIBLE){
            ((Window*)mAttachedView)->draw();
            GraphDevice::getInstance().flip();
        }
    }
    GraphDevice::getInstance().unlock();
#if COMPOSE_ASYNC
    if(GraphDevice::getInstance().needCompose())
        GraphDevice::getInstance().requestCompose();
#else
    GraphDevice::getInstance().composeSurfaces();
#endif
    GraphDevice::getInstance().lock();
    mRunnables.remove_if([](const RUNNER&r)->bool{
        return r.removed;
    });
    if(hasDelayedRunners()){
        //maybe user will removed runnable itself in its runnable'proc,so we use removed flag to flag it
        RUNNER runner=mRunnables.front();
        mRunnables.pop_front(); 
        if(runner.run)runner.run();
    }
    GraphDevice::getInstance().unlock();
    return 0;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
//codes between pragma will crashed in ubuntu GCC V8.x,bus GCC V7 wroked well.
bool UIEventSource::postDelayed(Runnable& run,uint32_t delayedtime){
    RUNNER runner;
    runner.removed=false;
    runner.time=SystemClock::uptimeMillis()+delayedtime;
    runner.run=run;
	
    for(auto itr=mRunnables.begin();itr!=mRunnables.end();itr++){
        if(runner.time<itr->time){
            mRunnables.insert(itr,runner);
            return true;
        }
    }
    mRunnables.push_back(runner);
    return true;
}
#pragma GCC pop_options

bool UIEventSource::hasDelayedRunners()const{
    if(mRunnables.empty())return false;
    nsecs_t nowms=SystemClock::uptimeMillis();
    RUNNER runner=mRunnables.front();
    return runner.time<nowms;
}

int UIEventSource::removeCallbacks(const Runnable& what){
    int count=0;
    for(auto it=mRunnables.begin();it!=mRunnables.end();it++){ 
        if((it->run==what)&&(it->removed==false)){
            it->removed=true;
            count++;
        }
    }
    return count;
}

}//end namespace
