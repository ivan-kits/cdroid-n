#ifndef __INPUT_DEVICE_H__
#define __INPUT_DEVICE_H__
#include <core/uievents.h>
#include <functional>
#include <map>
#include <memory>
#include <time.h>
namespace cdroid{
struct InputDeviceIdentifier {
    InputDeviceIdentifier() :
            bus(0), vendor(0), product(0), version(0) {
    }
    // Information provided by the kernel.
    std::string name;
    std::string location;
    std::string uniqueId;
    uint16_t bus;
    uint16_t vendor;
    uint16_t product;
    uint16_t version;

    // A composite input device descriptor string that uniquely identifies the device
    // even across reboots or reconnections.  The value of this field is used by
    // upper layers of the input system to associate settings with individual devices.
    // It is hashed from whatever kernel provided information is available.
    // Ideally, the way this value is computed should not change between Android releases
    // because that would invalidate persistent settings that rely on it.
    std::string descriptor;

    // A value added to uniquely identify a device in the absence of a unique id. This
    // is intended to be a minimum way to distinguish from other active devices and may
    // reuse values that are not associated with an input anymore.
    uint16_t nonce;
};

class InputDeviceInfo {
public:
    InputDeviceInfo();
    InputDeviceInfo(const InputDeviceInfo& other);
    ~InputDeviceInfo();

    struct MotionRange {
        int32_t axis;
        uint32_t source;
        float min;
        float max;
        float flat;
        float fuzz;
        float resolution;
    };

    void initialize(int32_t id, int32_t generation, int32_t controllerNumber, 
            const InputDeviceIdentifier& identifier,const std::string& alias, bool isExternal,bool hasMic);

    inline int32_t getId() const { return mId; }
    inline int32_t getControllerNumber() const { return mControllerNumber; }
    inline int32_t getGeneration() const { return mGeneration; }
    inline const InputDeviceIdentifier& getIdentifier() const { return mIdentifier; }
    inline const std::string& getAlias() const { return mAlias; }
    inline const std::string& getDisplayName() const {
        return mAlias.empty() ? mIdentifier.name : mAlias;
    }
    inline bool isExternal() const { return mIsExternal; }
    inline bool hasMic() const { return mHasMic; }
    inline uint32_t getSources() const { return mSources; }

    const MotionRange* getMotionRange(int32_t axis, uint32_t source) const;

    void addSource(uint32_t source);
    void addMotionRange(int32_t axis,uint32_t source,float min, float max,float flat,float fuzz,float resolution);
    void addMotionRange(const MotionRange& range);

    inline void setKeyboardType(int32_t keyboardType) { mKeyboardType = keyboardType; }
    inline int32_t getKeyboardType() const { return mKeyboardType; }

    inline void setVibrator(bool hasVibrator) { mHasVibrator = hasVibrator; }
    inline bool hasVibrator() const { return mHasVibrator; }

    inline void setButtonUnderPad(bool hasButton) { mHasButtonUnderPad = hasButton; }
    inline bool hasButtonUnderPad() const { return mHasButtonUnderPad; }

    inline const std::vector<MotionRange>& getMotionRanges() const {
        return mMotionRanges;
    }

private:
    int32_t mId;
    int32_t mGeneration;
    int32_t mControllerNumber;
    InputDeviceIdentifier mIdentifier;
    std::string mAlias;
    bool mIsExternal;
    bool mHasMic;
    uint32_t mSources;
    int32_t mKeyboardType;
    bool mHasVibrator;
    bool mHasButtonUnderPad;

    std::vector<MotionRange> mMotionRanges;
};

/* Input device classes. */
enum {
    /* The input device is a keyboard or has buttons. */
    INPUT_DEVICE_CLASS_KEYBOARD      = 0x00000001,

    /* The input device is an alpha-numeric keyboard (not just a dial pad). */
    INPUT_DEVICE_CLASS_ALPHAKEY      = 0x00000002,

    /* The input device is a touchscreen or a touchpad (either single-touch or multi-touch). */
    INPUT_DEVICE_CLASS_TOUCH         = 0x00000004,

    /* The input device is a cursor device such as a trackball or mouse. */
    INPUT_DEVICE_CLASS_CURSOR        = 0x00000008,

    /* The input device is a multi-touch touchscreen. */
    INPUT_DEVICE_CLASS_TOUCH_MT      = 0x00000010,

    /* The input device is a directional pad (implies keyboard, has DPAD keys). */
    INPUT_DEVICE_CLASS_DPAD          = 0x00000020,

    /* The input device is a gamepad (implies keyboard, has BUTTON keys). */
    INPUT_DEVICE_CLASS_GAMEPAD       = 0x00000040,

    /* The input device has switches. */
    INPUT_DEVICE_CLASS_SWITCH        = 0x00000080,

    /* The input device is a joystick (implies gamepad, has joystick absolute axes). */
    INPUT_DEVICE_CLASS_JOYSTICK      = 0x00000100,

    /* The input device has a vibrator (supports FF_RUMBLE). */
    INPUT_DEVICE_CLASS_VIBRATOR      = 0x00000200,

    /* The input device has a microphone. */
    INPUT_DEVICE_CLASS_MIC           = 0x00000400,

    /* The input device is an external stylus (has data we want to fuse with touch data). */
    INPUT_DEVICE_CLASS_EXTERNAL_STYLUS = 0x00000800,

    /* The input device has a rotary encoder */
    INPUT_DEVICE_CLASS_ROTARY_ENCODER = 0x00001000,

    /* The input device is virtual (not a real device, not part of UI configuration). */
    INPUT_DEVICE_CLASS_VIRTUAL       = 0x40000000,

    /* The input device is external (not built-in). */
    INPUT_DEVICE_CLASS_EXTERNAL      = 0x80000000,
};
extern uint32_t getAbsAxisUsage(int32_t axis, uint32_t deviceClasses);

class InputDevice{
public:
    static constexpr int SOURCE_CLASS_MASK     = 0x000000ff;
    static constexpr int SOURCE_CLASS_NONE     = 0x00000000;
    static constexpr int SOURCE_CLASS_BUTTON   = 0x00000001;
    static constexpr int SOURCE_CLASS_POINTER  = 0x00000002;
    static constexpr int SOURCE_CLASS_TRACKBALL= 0x00000004;
    static constexpr int SOURCE_CLASS_POSITION = 0x00000008;
    static constexpr int SOURCE_CLASS_JOYSTICK = 0x00000010;
    static constexpr int SOURCE_UNKNOWN  = 0x00000000;
    static constexpr int SOURCE_KEYBOARD = 0x00000100 | SOURCE_CLASS_BUTTON;
    static constexpr int SOURCE_DPAD = 0x00000200 | SOURCE_CLASS_BUTTON;
    static constexpr int SOURCE_GAMEPAD = 0x00000400 | SOURCE_CLASS_BUTTON;
    static constexpr int SOURCE_TOUCHSCREEN = 0x00001000 | SOURCE_CLASS_POINTER;
    static constexpr int SOURCE_MOUSE = 0x00002000 | SOURCE_CLASS_POINTER;
    static constexpr int SOURCE_STYLUS = 0x00004000 | SOURCE_CLASS_POINTER;
    static constexpr int SOURCE_BLUETOOTH_STYLUS =   0x00008000 | SOURCE_STYLUS;
    static constexpr int SOURCE_TRACKBALL = 0x00010000 | SOURCE_CLASS_TRACKBALL;
    static constexpr int SOURCE_MOUSE_RELATIVE = 0x00020000 | SOURCE_CLASS_TRACKBALL;
    static constexpr int SOURCE_TOUCHPAD = 0x00100000 | SOURCE_CLASS_POSITION;
    static constexpr int SOURCE_TOUCH_NAVIGATION = 0x00200000 | SOURCE_CLASS_NONE;
    static constexpr int SOURCE_ROTARY_ENCODER = 0x00400000 | SOURCE_CLASS_NONE;
    static constexpr int SOURCE_JOYSTICK = 0x01000000 | SOURCE_CLASS_JOYSTICK;
    static constexpr int SOURCE_HDMI = 0x02000000 | SOURCE_CLASS_BUTTON;
    static constexpr int SOURCE_ANY = 0xffffff00;    
    typedef std::function<void(const InputEvent&)>EventListener;
protected:
    int mDeviceClasses;
    unsigned int mScreenWidth;
    unsigned int mScreenHeight;
    InputDeviceInfo mDeviceInfo;
    EventListener listener;
    class KeyLayoutMap*kmap;
    virtual int isValidEvent(int type,int code,int value);
public:
    InputDevice(int fdev);
    virtual int putRawEvent(const struct timeval&tv,int type,int code,int value){return 0;}//PENDING need more rawevent OK,wecan getevent now
    void setEventConsumeListener(EventListener ls){listener=ls;}
    int getId()const;
    int getSource()const;
    int getVendor()const;
    int getProduct()const;
    int getClasses()const;
    const std::string&getName()const;
};

class KeyDevice:public InputDevice{
private:
    int mLastDownKey;
    int mRepeatCount;
protected:
    int msckey;
    KeyEvent mEvent;
    nsecs_t mDownTime;
public:
    KeyDevice(int fd);
    virtual int putRawEvent(const struct timeval&tv,int type,int code,int value);
};

class TouchDevice:public InputDevice{
protected:
    MotionEvent mEvent;
    nsecs_t mDownTime;
    nsecs_t mMoveTime;
    int mPointSlot;
    int mTPWidth;
    int mTPHeight;
    int mRangeXMin;
    int mRangeYMin;
    typedef struct{
        PointerCoords coord;
        PointerProperties prop; 
    }TouchPoint;
    std::map<int, TouchPoint>mPointMAP;
    void setAxisValue(int index,int axis,int value,bool isRelative);
public:
    TouchDevice(int fd);
    virtual int putRawEvent(const struct timeval&tv,int type,int code,int value);
};

class MouseDevice:public TouchDevice{
protected:
    uint8_t buttonstats[16];
public:
    MouseDevice(int fd);
    virtual int putRawEvent(const timeval&tv,int type,int code,int value);
};
}//namespace
#endif 
