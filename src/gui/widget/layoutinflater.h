#ifndef __LAYOUT_INFLATE_H__
#define __LAYOUT_INFLATE_H__
#include <widget/view.h>

namespace cdroid{
class LayoutInflater{
public:
    typedef std::function<View*(Context*ctx, const AttributeSet&attrs)>ViewInflater;
private:
    Context*mContext;
    LayoutInflater(Context*ctx);
    static std::map<const std::string,ViewInflater>mViewInflaters;
public:
    static LayoutInflater*from(Context*context);
    static ViewInflater getViewInflater(const std::string&);
    View* inflate(const std::string&resource,ViewGroup* root);
    View* inflate(const std::string&resource,ViewGroup* root, bool attachToRoot);
    View* inflate(std::istream&stream);
    View* inflate(const std::string&res);
};

}//endof namespace
#endif