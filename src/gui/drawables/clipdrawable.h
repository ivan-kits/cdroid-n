#ifndef __CLIP_DRAWABLE_H__
#define __CLIP_DRAWABLE_H__
#include <drawables/drawablewrapper.h>
namespace cdroid{

class ClipDrawable:public DrawableWrapper{
private:
    class ClipState:public DrawableWrapperState{
    public:
        int mOrientation;
        int mGravity;
        ClipState();
        ClipState(const ClipState& state);
        Drawable*newDrawable()override;
    };
    std::shared_ptr<ClipState>mState;
    ClipDrawable(std::shared_ptr<ClipState>state);
protected:
    bool onLevelChange(int level)override;
    std::shared_ptr<DrawableWrapperState> mutateConstantState()override;
public:
    enum Orientation{
        HORIZONTAL=1,
        VERTICAL  =2
    };
    ClipDrawable();
    ClipDrawable(Drawable* drawable, int gravity, int orientation);
    int getOpacity()override;
    int getGravity()const;
    int getOrientation()const;
    void draw(Canvas& canvas)override;
    static Drawable*inflate(Context*ctx,const AttributeSet&atts);
};

}
#endif
