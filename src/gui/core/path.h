#ifndef __CDROID_PATH_H__
#define __CDROID_PATH_H__
#include <core/rect.h>
#include <cairomm/context.h>
#include <vector>
namespace cdroid{

class Path{
private:
    Cairo::RefPtr<Cairo::Context>mCTX;     
public:
    Path();
    Path(const Path&);
    void append_to_context(Cairo::Context*)const;
    void append_to_context(const Cairo::RefPtr<Cairo::Context>&to)const;
    void move_to(double x, double y);
    void line_to(double x, double y);
    void rel_move_to(double x,double y);
    void rel_line_to(double,double);
    void set_fill_rule(Cairo::Context::FillRule fill_rule);
    void curve_to(double x1, double y1, double x2, double y2, double x3, double y3);
    void quad_to(double x1, double y1, double x2, double y2);
    void rel_curve_to(double x1, double y1, double x2, double y2, double x3, double y3);;
    void rel_quad_to(double dx1, double dy1, double dx2, double dy2);
    void arc(double xc, double yc, double radius, double angle1, double angle2);
    void arc_to(double x1, double y1, double x2, double y2, double radius);
    void arc_to(double rx, double ry, double angle, bool largeArc, bool sweepFlag, double x, double y);
    void rectangle(double x, double y, double width, double height);
    void round_rectangle(double x,double y,double width,double height,const std::vector<float>& radii);
    void round_rectangle(const RectF&rect,const std::vector<float>& radii);
    void reset();
    void begin_new_sub_path();
    void close_path();
    void approximate(std::vector<float>&,float acceptableError);
};
}
#endif
