#include "widget.h"
#include <QApplication>
#include <QTextCodec>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //解决中文显示问题
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);

    //设置中文字库
    int id = QFontDatabase::addApplicationFont("/opt/qt5.7.0/lib/fonts/DroidSansFallback.ttf");
    QString msyh = QFontDatabase::applicationFontFamilies (id).at(0);
    QFont font(msyh,10);
    font.setPointSize(10);
    a.setFont(font);

    Widget w;
    w.show();

    return a.exec();
}
