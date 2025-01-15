#include "widget.h"

#include <QApplication>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

bool loadDepend()
{
    QDir dir(QCoreApplication::applicationDirPath());
    do {
        if(!Widget::saveResFile(":/dumpbin.exe",
            dir.absoluteFilePath("dumpbin.exe")))
            break;
        if(!Widget::saveResFile(":/link.exe",
            dir.absoluteFilePath("link.exe")))
            break;
        if(!Widget::saveResFile(":/tbbmalloc.dll",
            dir.absoluteFilePath("tbbmalloc.dll")))
            break;
        if(!Widget::saveResFile(":/mspdbcore.dll",
            dir.absoluteFilePath("mspdbcore.dll")))
            break;
        if(!Widget::saveResFile(":/mspdb140.dll",
            dir.absoluteFilePath("mspdb140.dll")))
            break;
        return true;
    } while(0);
    dir.remove("dumpbin.exe");
    dir.remove("link.exe");
    dir.remove("tbbmalloc.dll");
    return false;
}
#include <QTranslator>
#include <qdarkstyle.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //加载文本
    QTranslator translator;
    translator.load(":/TextEditMenuCN.qm");
    qApp->installTranslator(&translator);
    //设置样式
    SetQDarkStyleSheet();
//    QFile f(":/style.qss");
//    if (f.exists())
//    {
//        f.open(QFile::ReadOnly | QFile::Text);
//        QTextStream ts(&f);
//        qApp->setStyleSheet(ts.readAll());
//        f.close();
//    }
    //加载依赖程序
    if(!loadDepend()) {
        //加载程序失败
        QMessageBox::information(nullptr, u8"提示", u8"资源文件加载失败", u8"确定");
        return 0;
    }
    //启动程序
    Widget w;
    w.show();
    return a.exec();
}
