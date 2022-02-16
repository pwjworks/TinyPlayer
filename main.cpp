#include "tinyplayer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TinyPlayer w;
    w.show();
    return a.exec();
}
