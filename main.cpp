#include "ltm.h"
#include <QFile>
#include <QTextStream>
#include "cpustats.h"
#include <QThread>

#include <QApplication>

int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    LTM w;

    QFile f(":qdarkstyle/style.qss");
    if (!f.exists())
    {
        //std::cout << std::endl;
        //printf("Unable to set stylesheet, file not found\n");
    }
    else
    {
        const QFlags<QIODevice::OpenModeFlag> flags = {QIODevice::OpenModeFlag::Text,QIODevice::OpenModeFlag::ReadOnly};
        QFile::OpenMode mode = QIODevice::OpenMode(flags);
        f.open(mode);
        QTextStream ts(&f);
        qApp->setStyleSheet(ts.readAll());
      //  qApp->setStyleSheet(
   //                         "QWidget {background-color: #19232D;  border: 0px solid #32414B;  padding: 0px; color: #F0F0F0; selection-background-color: #1464A0;   selection-color: #F0F0F0;  }");
    }

//CPUStats::get_cpus_activity();
//QThread::msleep(100);
//auto vec = CPUStats::get_cpus_activity();


    w.show();
    return a.exec();
}
