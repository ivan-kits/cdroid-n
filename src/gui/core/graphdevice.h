#ifndef __GRAPH_DEVICE_H__
#define __GRAPH_DEVICE_H__
#include <core/rect.h>
#include <cairomm/context.h>
#include <mutex>
#include <condition_variable>
#include <map>

#ifndef COMPOSE_ASYNC
#define COMPOSE_ASYNC 0
#endif

namespace cdroid{
class GraphDevice{
private:
    int mScreenWidth;
    int mScreenHeight;
    int mFormat;
    int mComposing;
    int mPendingCompose;
    bool mQuitFlag;
    bool mShowFPS;
    uint64_t mLastComposeTime;
    uint64_t mFpsStartTime;
    uint64_t mFpsPrevTime;
    uint64_t mFpsNumFrames;
    Rect mRectBanner;
    std::mutex mMutex;
    std::condition_variable mCV;
    std::string mFPSText;
    HANDLE mPrimarySurface;
    class Canvas*mPrimaryContext;
    //Cairo::RefPtr<Cairo::Region>mInvalidateRgn;
    static GraphDevice*mInst;
    GraphDevice(int format=-1);
    void trackFPS(Canvas&);
    void doCompose();
    void computeVisibleRegion(std::vector<class Window*>&windows,std::vector<Cairo::RefPtr<Cairo::Region>>&regions);
    void rotateRectInWindow(const Rect&rcw,const Rect&rs,Rect&rd,int&dx,int&dy,int rotation);
public:
    static GraphDevice&getInstance();
    ~GraphDevice();
    void getScreenSize(int &w,int&h);
    int getScreenWidth();
    int getScreenHeight();
    void flip();
    void requestCompose();
    void lock();
    void unlock();
    void composeSurfaces();
    bool needCompose();
    Canvas*getPrimaryContext();
    void showFPS(bool);
    void invalidate(const Rect&);
    HANDLE getPrimarySurface()const;
};
}
#endif

