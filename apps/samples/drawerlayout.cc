#include<windows.h>
#include<widget/listview.h>
#include<widget/drawerlayout.h>
#include<cdlog.h>

class MyAdapter:public ArrayAdapter<std::string>{
public:
    MyAdapter():ArrayAdapter(){
    }
    View*getView(int position, View* convertView, ViewGroup* parent)override{
        TextView*tv=(TextView*)convertView;
        if(convertView==nullptr){
            tv=new TextView("",600,20);
            tv->setPadding(20,0,0,0);
            tv->setFocusable(false);
        }
        tv->setId(position);
        tv->setText("position :"+std::to_string(position));
        tv->setTextColor(0xFFFFFFFF);
        tv->setBackgroundColor(0x80002222);
        tv->setTextSize(40);
        return tv;
    }
};

int main(int argc,const char*argv[]){
    App app(argc,argv);
    Window*w=new Window(0,0,1280,720);

    DrawerLayout*dl=new DrawerLayout(1280,720);


    /*content area*/
    LinearLayout*content=new LinearLayout(1280,720);
    content->setOrientation(LinearLayout::VERTICAL);
    TextView*tv=new TextView("TextView",40,40);
    content->setBackgroundColor(0xFFFF0000);
    tv->setTextSize(60);
    content->addView(tv).setId(100);

    ShapeDrawable*sd=new ShapeDrawable();
    sd->setShape(new ArcShape(0,360));
    sd->getShape()->setSolidColor(0x800000FF);
    RippleDrawable*rp=new RippleDrawable(ColorStateList::valueOf(0x80222222),
       new ColorDrawable(0x8000FF00),sd);
    Button*btn=new Button("Open",100,64);
    btn->setMinimumHeight(128);
    btn->setBackgroundDrawable(rp);
    content->addView(btn);
    btn->setOnClickListener([dl](View&){
        LOGD("openDrawer");
        dl->openDrawer(Gravity::START);
    });

    btn=new Button("Close",100,64);
    content->addView(btn);
    btn->setOnClickListener([dl](View&){
        LOGD("closeDrawer");
        dl->closeDrawer(Gravity::START);
    });
   
    dl->setBackgroundColor(0xFF111111);
    DrawerLayout::LayoutParams*lp=new  DrawerLayout::LayoutParams(LayoutParams::MATCH_PARENT,LayoutParams::MATCH_PARENT);
    lp->gravity=Gravity::NO_GRAVITY;
    dl->addView(content,0,lp);


    /**create left slider*/
    lp=new  DrawerLayout::LayoutParams(240,LayoutParams::MATCH_PARENT);
    lp->gravity=Gravity::START;
    LinearLayout*left=new LinearLayout(0,0);
    left->setOrientation(LinearLayout::VERTICAL);
    left->setBackgroundColor(0xFF00FF00);
    left->setZ(100);

    MyAdapter*adapter=new MyAdapter();
    ListView*lv=(ListView*)&left->addView(new ListView(460,500));
    lv->setPos(10,10);
    for(int i=0;i<56;i++){
        adapter->add("");
    }
    lv->setAdapter(adapter);
    adapter->notifyDataSetChanged();
    lv->setVerticalScrollBarEnabled(true);
    lv->setOverScrollMode(View::OVER_SCROLL_ALWAYS);
    lv->setSmoothScrollbarEnabled(true);
    lv->setSelector(new ColorDrawable(0x8800FF00));
    lv->setDivider(new ColorDrawable(0x80224422));
    lv->setDividerHeight(1);

    dl->addView(left,1,lp);

    /*right slider*/ 
    LinearLayout*right=new LinearLayout(300,720);
    right->setOrientation(LinearLayout::VERTICAL);
    tv=new TextView("Right Panel",40,40);
    tv->setTextSize(40);
    right->addView(tv);
    right->setBackgroundColor(0x80222222);
    lp=new DrawerLayout::LayoutParams(320,LayoutParams::MATCH_PARENT);
    lp->gravity=Gravity::END;
    dl->addView(right,2,lp);

    w->addView(dl);
    dl->requestLayout();
    app.exec();
}
