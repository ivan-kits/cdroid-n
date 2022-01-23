#include <fileadapter.h>
#include <dirent.h>
#include <cdroid.h>
namespace cdroid{
std::string SimplifyPath(const std::string & path) {
    char rpath[1024];
    realpath(path.c_str(),rpath);
    return std::string(rpath);
}

static int file_filter(const struct dirent*ent){
    return (ent->d_type==DT_REG)||(ent->d_type==DT_DIR);
}

View*FileAdapter::getView(int position, View* convertView, ViewGroup* parent){
    const FileItem& mi=getItemAt(position);
    ViewGroup*vp=(ViewGroup*)convertView;
    TextView*tv;
    if(convertView==nullptr){
        vp=(ViewGroup*)LayoutInflater::from(&App::getInstance())->inflate("layout/fileitem.xml",nullptr);
    }
    tv=(TextView*)vp->findViewById(1);
    tv->setText(std::to_string(position));
    tv=(TextView*)vp->findViewById(2);
    tv->setText(mi.fileName);
    tv->setTextColor(mi.isDir?0xFFFFFFFF:0xFF88FF00);
    tv->setTextSize(28);
    return vp;
}

int FileAdapter::loadFiles(const std::string&filepath){
    std::string path=SimplifyPath(filepath);
    DIR*dir=opendir(filepath.c_str());
    struct dirent*ent;
    int count=0;
    while((ent=readdir(dir))!=nullptr){
        FileItem mi;
        mi.fileName=ent->d_name;
        mi.fullpath=SimplifyPath(path+"/"+mi.fileName);
        LOGV("%s",mi.fullpath.c_str());
        switch(ent->d_type){
        case DT_DIR:
        case DT_REG:
            mi.isDir=(ent->d_type==DT_DIR);
            add(mi);
            count++;
            break;
        default:break;
        }
    }
    if(dir)closedir(dir);
    LOGV("%s scaned=%d",filepath.c_str(),count);
    return count;
}

std::string FileAdapter::SimplifyPath(const std::string & path) {
    char rpath[1024];
    realpath(path.c_str(),rpath);
    return std::string(rpath);
}

FileItem::FileItem(){
    fileSize=0;
    isDir=false;
}
FileItem::FileItem(const FileItem&o){
    isDir=o.isDir;
    fileName=o.fileName;
    fullpath=o.fullpath;
}

}
