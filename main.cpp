#include "ltm.h"
#include <QFile>
#include <QTextStream>
#include "cpustats.h"
#include <QThread>

#include <QApplication>

int main(int argc, char *argv[])
{
      std::cout << "setting auto DPI screen scale factor" << std::endl;
    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");

//    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    std::cout << "instantiating QApplication" << std::endl;
    QApplication a(argc, argv);


   // QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    std::cout << "instantiating MainWindow" << std::endl;
    LTM w;


     std::cout << "loading qDarkStylesheet..." << std::endl;
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


    try {
        std::cout << "showing QMainWindow" << std::endl;
        w.show();

    } catch (std::exception& e) {
       std::cerr << "execption caught in main when showing main window: " << e.what()<< std::endl;
       w.show();
    } catch (...){
        std::cerr << "unknown execption in main when showing main window: "<< std::endl;
        w.show();
    }



    std::cout << "executing QApp eventloop" << std::endl;
    int exRetVal = 0;

       try {
   exRetVal = a.exec();
    } catch (std::exception& e) {
       std::cerr << "execption caught in main when executing event loop: " << e.what()<< std::endl;
       exRetVal = a.exec();
    } catch (...){
        std::cerr << "unknown execption in main when executing event loop: "<< std::endl;
        exRetVal = a.exec();
    }


    return exRetVal;
}
