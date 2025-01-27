#include <cdtypes.h>
#include <ngl_os.h>
#include <cdgraph.h>
#include <cdinput.h>
#include <cdlog.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <pthread.h>
#include <string.h>
#include <core/eventcodes.h>
#include <time.h>

static Display*x11Display=NULL;
static Window x11Window=0;
static Visual *x11Visual=NULL;
static Atom WM_DELETE_WINDOW;
static GC mainGC=0;
static XImage*mainSurface=NULL;
static void* X11EventProc(void*p);
static GFXRect screenMargin= {0}; //{60,0,60,0};

#define SENDKEY(k,down) {InjectKey(EV_KEY,k,down);}
#define SENDMOUSE(time,x,y)  {InjectABS(time,EV_ABS,0,x);\
            InjectABS(time,EV_ABS,1,y);InjectABS(time,EV_SYN,SYN_REPORT,0);}

static void InjectKey(int type,int code,int value) {
    INPUTEVENT i= {0};
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    i.tv_sec=ts.tv_sec;
    i.tv_usec=ts.tv_nsec/1000;
    i.type=type;
    i.code=code;
    i.value=value;
    i.device=INJECTDEV_KEY;
    InputInjectEvents(&i,1,1);
}
static void InjectABS(ULONG time,int type,int axis,int value) {
    INPUTEVENT i= {0};
    i.tv_sec=time/1000;
    i.tv_usec=(time%1000)*1000;
    i.type=type;
    i.code=axis;
    i.value=value;
    i.device=INJECTDEV_TOUCH;
    InputInjectEvents(&i,1,1);
}
static void onExit() {
    LOGD("X11 Graph shutdown!");
    if(x11Display) {
        XDestroyWindow(x11Display,x11Window);
        XCloseDisplay(x11Display);
        x11Display=NULL;
    }
}

INT GFXInit() {
    if(x11Display)return E_OK;
    XInitThreads();
    x11Display=XOpenDisplay(NULL);
    LOGI("x11Display init =%p",x11Display);
    if(x11Display) {
        pthread_t tid;
        XSetWindowAttributes winattrs;
        XGCValues values;
        XSizeHints sizehints;
        int width,height;
        int screen=DefaultScreen(x11Display);
        x11Visual = DefaultVisual(x11Display, screen);
        GFXGetDisplaySize(0,&width,&height);
        width += screenMargin.x + screenMargin.w;
        height+= screenMargin.y + screenMargin.h;
        x11Window=XCreateSimpleWindow(x11Display, RootWindow(x11Display, screen), 0, 0,width,height,
                                      1, BlackPixel(x11Display, screen), WhitePixel(x11Display, screen));

        sizehints.flags = PMinSize | PMaxSize;
        sizehints.min_width = width;
        sizehints.max_width = width;
        sizehints.min_height = height;
        sizehints.max_height = height;
        XSetWMNormalHints(x11Display,x11Window,&sizehints);

        WM_DELETE_WINDOW = XInternAtom(x11Display, "WM_DELETE_WINDOW", False);
        XSetWMProtocols(x11Display,x11Window, &WM_DELETE_WINDOW, 1);
        mainGC = XCreateGC(x11Display,x11Window,0, &values);
        XSelectInput(x11Display, x11Window, ExposureMask | KeyPressMask|KeyReleaseMask |ResizeRedirectMask|
                     ButtonPressMask | ButtonReleaseMask | PointerMotionMask | Button1MotionMask | Button2MotionMask );
        XMapWindow(x11Display,x11Window);
        pthread_create(&tid,NULL,X11EventProc,NULL);
    }
    atexit(onExit);
    return E_OK;
}

INT GFXGetDisplaySize(int dispid,UINT*width,UINT*height) {
    const char*env= getenv("SCREENSIZE");
    if(env==NULL) {
        *width=1280;//dispCfg.width;
        *height=720;//dispCfg.height;
    } else {
        *width=atoi(env)- screenMargin.x - screenMargin.w;
        env=strpbrk(env,"x*,");
        if((*width<=0)||(env==NULL))exit(-1);
        *height=atoi(env+1)- screenMargin.y - screenMargin.h;
        if(*height<=0)exit(-1);
    }
    return E_OK;
}

INT GFXGetDisplayCount() {
    return 1;
}

INT GFXLockSurface(HANDLE surface,void**buffer,UINT*pitch) {
    XImage*img=(XImage*)surface;
    *buffer=img->data;
    *pitch=img->bytes_per_line;
    return 0;
}

INT GFXGetSurfaceInfo(HANDLE surface,UINT*width,UINT*height,INT *format) {
    XImage*img=(XImage*)surface;
    *width=img->width;
    *height=img->height;
    if(format)*format=GPF_ARGB;
    return E_OK;
}

INT GFXUnlockSurface(HANDLE surface) {
    return 0;
}

INT GFXSurfaceSetOpacity(HANDLE surface,BYTE alpha) {
    return 0;//dispLayer->SetOpacity(dispLayer,alpha);
}

static void  X11Expose(int x,int y,int w,int h) {
    XExposeEvent e;
    memset(&e,0,sizeof(e));
    e.type = Expose;
    e.send_event=True;
    e.display= x11Display;
    e.window = x11Window;
    e.x = x ;
    e.y = y ;
    e.width = w;
    e.height=h;
    e.count=1;
    if(x11Display&&mainSurface) {
        XPutImage(x11Display,x11Window,mainGC,mainSurface,x,y,x,y,w,h);
        //XSendEvent(x11Display,x11Window,False,0,(XEvent*)&e);
        //XPutBackEvent(x11Display,(XEvent*)&e);
    }
}
INT GFXFillRect(HANDLE surface,const GFXRect*rect,UINT color) {
    XImage*img=(XImage*)surface;
    UINT x,y;
    GFXRect rec= {0,0,0,0};
    rec.w=img->width;
    rec.h=img->height;
    if(rect)rec=*rect;
    LOGV("FillRect %p %d,%d-%d,%d color=0x%x pitch=%d",img,rec.x,rec.y,rec.w,rec.h,color,img->bytes_per_line);
    UINT*fb=(UINT*)(img->data+img->bytes_per_line*rec.y+rec.x*4);
    UINT*fbtop=fb;
    for(x=0; x<rec.w; x++)fb[x]=color;
    for(y=1; y<rec.h; y++) {
        fb+=(img->bytes_per_line>>2);
        memcpy(fb,fbtop,rec.w*4);
    }
    if(surface==mainSurface) {
        X11Expose(rec.x,rec.y,rec.w,rec.h);
    }
    return E_OK;
}

INT GFXFlip(HANDLE surface) {
    XImage *img=(XImage*)surface;
    if(mainSurface==surface) {
        GFXRect rect= {0,0,img->width,img->height};
        //X11Expose(0,0,img->width,img->height);
    }
    return 0;
}

INT GFXCreateSurface(int dispid,HANDLE*surface,UINT width,UINT height,INT format,BOOL hwsurface) {
    XImage*img=NULL;
    if(x11Display) {
        int imagedepth = DefaultDepth(x11Display,DefaultScreen(x11Display));
        if(hwsurface) {
            width += screenMargin.x + screenMargin.w;
            height+= screenMargin.y + screenMargin.h;
        }
        img=XCreateImage(x11Display,x11Visual, imagedepth,ZPixmap,0,NULL,width,height,32,0);
    } else {
        img=(XImage*)malloc(sizeof(XImage));
        img->width=width;
        img->height=height;
        img->bits_per_pixel=24;
        img->bytes_per_line=width*4;
    }
    img->data=(char*)malloc(width*height*img->bytes_per_line);
    *surface=img;
    LOGD("%p  size=%dx%dx%d %db",img,width,height,img->bytes_per_line,img->bits_per_pixel);
    if(hwsurface) {
        mainSurface=img;
    }
    return E_OK;
}


INT GFXBlit(HANDLE dstsurface,int dx,int dy,HANDLE srcsurface,const GFXRect* srcrect) {
    XImage *ndst=(XImage*)dstsurface;
    XImage *nsrc=(XImage*)srcsurface;
    GFXRect rs= {0,0};
    BYTE*pbs=(BYTE*)nsrc->data;
    BYTE*pbd=(BYTE*)ndst->data;
    rs.w=nsrc->width;
    rs.h=nsrc->height;
    if(srcrect)rs=*srcrect;
    if(((int)rs.w+dx<=0)||((int)rs.h+dy<=0)||(dx>=(int)ndst->width)||(dy>=(int)ndst->height)||(rs.x<0)||(rs.y<0)) {
        LOGD("dx=%d,dy=%d rs=(%d,%d-%d,%d)",dx,dy,rs.x,rs.y,rs.w,rs.h);
        return E_INVALID_PARA;
    }

    if(dx<0) {
        rs.x -= dx;
        rs.w = (int)rs.w+dx;
        dx=0;
    }
    if(dy<0) {
        rs.y -= dy;
        rs.h = (int)rs.h+dy;
        dy=0;
    }
    if(dx + rs.w > ndst->width -screenMargin.x - screenMargin.w) rs.w = ndst->width -screenMargin.x - screenMargin.w -dx;
    if(dy + rs.h > ndst->height-screenMargin.y - screenMargin.h) rs.h = ndst->height-screenMargin.y - screenMargin.h -dy;

    //LOGV_IF(ndst==mainSurface,"Blit %p %d,%d-%d,%d -> %p %d,%d buffer=%p->%p",nsrc,rs.x,rs.y,rs.w,rs.h,ndst,dx,dy,pbs,pbd);
    pbs += rs.y*nsrc->bytes_per_line+rs.x*4;
    pbd += dy*ndst->bytes_per_line+dx*4;
    if(ndst==mainSurface) {
        pbd += (screenMargin.x*4 + screenMargin.h*ndst->bytes_per_line);
    }
    for(unsigned int y=0; y<rs.h; y++) {
        memcpy(pbd,pbs,rs.w*4);
        pbs+=nsrc->bytes_per_line;
        pbd+=ndst->bytes_per_line;
    }
    LOGV("src (%d,%d,%d,%d) dst (%d,%d,%d,%d)",rs.x,rs.y,rs.w,rs.h,dx,dy,rs.w,rs.h);
    if((ndst==mainSurface)&&x11Display) {
        X11Expose(dx+screenMargin.x,dy+screenMargin.h,rs.w,rs.h);
    }
    return 0;
}

INT GFXDestroySurface(HANDLE surface) {
    XDestroyImage((XImage*)surface);
    return 0;
}

static void* X11EventProc(void*p) {
    XEvent event;
    int down;
    while(x11Display) {
        int rc=XNextEvent(x11Display, &event);
        switch(event.type) {
        case Expose:
            if(mainSurface) {
                XExposeEvent e=event.xexpose;
                XPutImage(x11Display,x11Window,mainGC,mainSurface,e.x,e.y,e.x,e.y,e.width,e.height);
            }
            break;
        case ConfigureNotify:
            XPutImage(x11Display,x11Window,mainGC,mainSurface,0,0,0,0,mainSurface->width,mainSurface->height);
            break;
        case KeyPress:/* 当检测到键盘按键,退出消息循环 */
        case KeyRelease:
            down=event.type==KeyPress;
            switch(event.xkey.keycode) {
            case 0x13:
                SENDKEY(KEY_0,down);
                break;
            case 0x0a ... 0x12:
                SENDKEY(KEY_1+event.xkey.keycode-0x0a,down);
                break;
            case 0x43 ... 0x4d:
                SENDKEY(KEY_F1+event.xkey.keycode-0x43,down);
                break;
            case 0x5F ... 0x60:
                SENDKEY(KEY_F11+event.xkey.keycode-0x5F,down);
                break;
            case 0x64 :
                SENDKEY(KEY_LEFT,down) ;
                break;
            case 0x66 :
                SENDKEY(KEY_RIGHT,down);
                break;
            case 0x61 :
                SENDKEY(KEY_HOME,down) ;
                break;
            case 0x62 :
                SENDKEY(KEY_UP,down)   ;
                break;
            case 0x63 :
                SENDKEY(KEY_PAGEUP,down);
                break;
            case 0x67 :
                SENDKEY(KEY_MOVE_END,down);
                break;
            case 0x68 :
                SENDKEY(KEY_DOWN,down) ;
                break;
            case 0x69 :
                SENDKEY(KEY_PAGEDOWN,down);
                break;
            case 0x6A :
                SENDKEY(KEY_INSERT,down);
                break;
            case 0x6B :
                SENDKEY(KEY_DELETE,down);
                break;
            case 0x6C :
                SENDKEY(KEY_DPAD_CENTER,down);
                break;
            case 0x09 :
                SENDKEY(KEY_ESCAPE,down);
                break;
            case 0x24 :
                SENDKEY(KEY_ENTER,down);
                break;
            case 0x41 :
                SENDKEY(KEY_SPACE,down);
                break;
            case 0x16 :
                SENDKEY(KEY_BACKSPACE,down);
                break;
            case 0x75 :
                SENDKEY(KEY_MENU,down) ;
                break;

            case 0x18 :
                SENDKEY(KEY_Q,down);
                break;
            case 0x19 :
                SENDKEY(KEY_W,down);
                break;
            case 0x1A :
                SENDKEY(KEY_E,down);
                break;
            case 0x1B :
                SENDKEY(KEY_R,down);
                break;
            case 0x1C :
                SENDKEY(KEY_T,down);
                break;
            case 0x1D :
                SENDKEY(KEY_Y,down);
                break;
            case 0x1E :
                SENDKEY(KEY_U,down);
                break;
            case 0x1F :
                SENDKEY(KEY_I,down);
                break;
            case 0x20 :
                SENDKEY(KEY_O,down);
                break;
            case 0x21 :
                SENDKEY(KEY_P,down);
                break;

            case 0x26 :
                SENDKEY(KEY_A,down);
                break;
            case 0x27 :
                SENDKEY(KEY_S,down);
                break;
            case 0x28 :
                SENDKEY(KEY_D,down);
                break;
            case 0x29 :
                SENDKEY(KEY_F,down);
                break;
            case 0x2A :
                SENDKEY(KEY_G,down);
                break;
            case 0x2B :
                SENDKEY(KEY_H,down);
                break;
            case 0x2C :
                SENDKEY(KEY_J,down);
                break;
            case 0x2D :
                SENDKEY(KEY_K,down);
                break;
            case 0x2E :
                SENDKEY(KEY_L,down);
                break;

            case 0x34 :
                SENDKEY(KEY_Z,down);
                break;
            case 0x35 :
                SENDKEY(KEY_X,down);
                break;
            case 0x36 :
                SENDKEY(KEY_C,down);
                break;
            case 0x37 :
                SENDKEY(KEY_V,down);
                break;
            case 0x38 :
                SENDKEY(KEY_B,down);
                break;
            case 0x39 :
                SENDKEY(KEY_N,down);
                break;
            case 0x3A :
                SENDKEY(KEY_M,down);
                break;
            }
            LOGV("%d",event.xkey.keycode);
            break;
        case ButtonPress:
        case ButtonRelease:
            if(1==event.xbutton.button) {
                InjectABS(event.xbutton.time,EV_KEY,BTN_TOUCH,(event.type==ButtonPress)?1:0);
                SENDMOUSE(event.xbutton.time,event.xbutton.x - screenMargin.x, event.xbutton.y - screenMargin.y);
            }
            break;
        case MotionNotify:
            if(event.xmotion.state==0x100) {
                SENDMOUSE(event.xmotion.time,event.xmotion.x - screenMargin.x, event.xmotion.y - screenMargin.y);
            }
            break;
        case DestroyNotify:
            LOGD("====X11Window::Destroied");
            return 0;
        case ClientMessage:
            if ( (Atom) event.xclient.data.l[0] == WM_DELETE_WINDOW) {
                LOGD("====ClientMessage WM_DELETE_WINDOW");
                LOGD("GraphX11.Terminated");
                exit(0);
            }
            break;
        case UnmapNotify:
            LOGD("===UnmapNotify ");
            break;
        default:
            LOGD("event.type=%d",event.type);
            break;
        };
    }
}

