#include "qdarkstyle.h"

#include <QApplication>
#include <QTranslator>
#include <QTextStream>
#include <QFile>

void SetQDarkStyleSheet(bool dark)
{
    QString styleFile;
    if(dark) {
        styleFile = ":/qdarkstyle/dark.qss";
    } else {
        styleFile = ":/qdarkstyle/light.qss";
    }
    QFile style(styleFile);
    if (style.open(QFile::ReadOnly | QFile::Text))
    {
        qApp->setStyleSheet(style.readAll());
        style.close();
    }
}
