#include <windows.h>
class MyPageAdapter:public PagerAdapter{
public:
    int getCount(){return 8;}
    bool isViewFromObject(View* view, void*object) { return view==object;}
    void* instantiateItem(ViewGroup* container, int position) {
        SimpleMonthView*sm=new  SimpleMonthView(1280,560);
        sm->setMonthParams(23,Calendar::MAY+position,2021,-1,1,31);
        ViewPager::LayoutParams*lp=new ViewPager::LayoutParams();
        lp->isDecor=false;//isDecor must be true for inflation view(create from xml layout),otherwise must be false
        container->addView(sm);
        return sm;
    }
    void destroyItem(ViewGroup* container, int position,void* object){
        container->removeView((View*)object);
    }
    std::string getPageTitle(int position){
        return std::string("Tab")+std::to_string(position);
    }
    float getPageWidth(int position){return 1.f;}//if returned calue <1 OffscreenPageLimit must be larger to workfine
};


int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,600);
    MyPageAdapter*gpAdapter=new MyPageAdapter();

    LinearLayout*layout=new LinearLayout(1280,600);
    layout->setOrientation(LinearLayout::VERTICAL);

    TabLayout* tab=new TabLayout(1280,36);
    ViewPager*pager=new ViewPager(1280,560);
    pager->setOffscreenPageLimit(8);
    pager->setAdapter(gpAdapter);
    pager->setBackgroundColor(0xFFF3333);
    
    tab->setupWithViewPager(pager);
    layout->addView(tab);
    layout->addView(pager).setId(10);
    w->addView(layout);
    w->requestLayout();
    app.exec();
}
