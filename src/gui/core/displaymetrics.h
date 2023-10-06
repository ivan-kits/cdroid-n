#ifndef __DISPLAY_METRICS_H__
#define __DISPLAY_METRICS_H__
namespace cdroid{

class DisplayMetrics{
public:
    static constexpr int DENSITY_LOW    = 120;
    static constexpr int DENSITY_140    = 140;
    static constexpr int DENSITY_MEDIUM = 160;
    static constexpr int DENSITY_180    = 180;
    static constexpr int DENSITY_200    = 200;
    static constexpr int DENSITY_TV     = 213;
    static constexpr int DENSITY_220    = 220;
    static constexpr int DENSITY_HIGH   = 240;
    static constexpr int DENSITY_260    = 260;
    static constexpr int DENSITY_280    = 280;
    static constexpr int DENSITY_300    = 300;
    static constexpr int DENSITY_XHIGH  = 320;
    static constexpr int DENSITY_340    = 340;
    static constexpr int DENSITY_360    = 360;
    static constexpr int DENSITY_400    = 400;
    static constexpr int DENSITY_420    = 420;
    static constexpr int DENSITY_440    = 440;
    static constexpr int DENSITY_450    = 450;	
    static constexpr int DENSITY_XXHIGH = 480;
    static constexpr int DENSITY_560    = 560;
    static constexpr int DENSITY_600    = 600;
    static constexpr int DENSITY_XXXHIGH= 640;
    static constexpr int DENSITY_DEFAULT= DENSITY_MEDIUM;
    static constexpr float DENSITY_DEFAULT_SCALE = 1.0f / DENSITY_DEFAULT;
    static int DENSITY_DEVICE;
    static int DENSITY_DEVICEE_STABLE;
public:
    int widthPixels;
    int heightPixels;
    float density;
    int densityDpi;
    float scaledDensity;
    float xdpi;
    float ydpi;
    int noncompatWidthPixels;
    int noncompatHeightPixels;
    float noncompatDensity;
    int noncompatDensityDpi;
    float noncompatScaledDensity;
    float noncompatXdpi;
    float noncompatYdpi;
public:
    DisplayMetrics();
    void setTo(const DisplayMetrics&);
    void setToDefaults();
    bool equals(const DisplayMetrics& other)const;
    bool equalsPhysical(const DisplayMetrics& other)const;
    static int getDeviceDensity();
};

}
#endif
