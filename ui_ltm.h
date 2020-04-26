/********************************************************************************
** Form generated from reading UI file 'ltm.ui'
**
** Created by: Qt User Interface Compiler version 5.14.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LTM_H
#define UI_LTM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "qwt_plot.h"

QT_BEGIN_NAMESPACE

class Ui_LTM
{
public:
    QWidget *centralwidget;
    QHBoxLayout *horizontalLayout;
    QListWidget *listWidget;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QwtPlot *plotCpu;
    QLabel *label_9;
    QPushButton *pushButton;
    QWidget *page_2;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *LTM)
    {
        if (LTM->objectName().isEmpty())
            LTM->setObjectName(QString::fromUtf8("LTM"));
        LTM->resize(1111, 759);
        centralwidget = new QWidget(LTM);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        horizontalLayout = new QHBoxLayout(centralwidget);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        listWidget = new QListWidget(centralwidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        new QListWidgetItem(listWidget);
        listWidget->setObjectName(QString::fromUtf8("listWidget"));
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(listWidget->sizePolicy().hasHeightForWidth());
        listWidget->setSizePolicy(sizePolicy);
        listWidget->setSelectionBehavior(QAbstractItemView::SelectItems);

        horizontalLayout->addWidget(listWidget);

        stackedWidget = new QStackedWidget(centralwidget);
        stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(2);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(stackedWidget->sizePolicy().hasHeightForWidth());
        stackedWidget->setSizePolicy(sizePolicy1);
        stackedWidget->setStyleSheet(QString::fromUtf8(""));
        page = new QWidget();
        page->setObjectName(QString::fromUtf8("page"));
        plotCpu = new QwtPlot(page);
        plotCpu->setObjectName(QString::fromUtf8("plotCpu"));
        plotCpu->setGeometry(QRect(10, 60, 701, 421));
        label_9 = new QLabel(page);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(11, 4, 101, 41));
        sizePolicy1.setHeightForWidth(label_9->sizePolicy().hasHeightForWidth());
        label_9->setSizePolicy(sizePolicy1);
        QFont font;
        font.setPointSize(26);
        font.setBold(true);
        font.setWeight(75);
        label_9->setFont(font);
        pushButton = new QPushButton(page);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(30, 510, 89, 25));
        stackedWidget->addWidget(page);
        page_2 = new QWidget();
        page_2->setObjectName(QString::fromUtf8("page_2"));
        stackedWidget->addWidget(page_2);

        horizontalLayout->addWidget(stackedWidget);

        LTM->setCentralWidget(centralwidget);
        menubar = new QMenuBar(LTM);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 1111, 22));
        LTM->setMenuBar(menubar);
        statusbar = new QStatusBar(LTM);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        LTM->setStatusBar(statusbar);

        retranslateUi(LTM);

        QMetaObject::connectSlotsByName(LTM);
    } // setupUi

    void retranslateUi(QMainWindow *LTM)
    {
        LTM->setWindowTitle(QCoreApplication::translate("LTM", "LTM", nullptr));

        const bool __sortingEnabled = listWidget->isSortingEnabled();
        listWidget->setSortingEnabled(false);
        listWidget->setSortingEnabled(__sortingEnabled);

        label_9->setText(QCoreApplication::translate("LTM", "CPU", nullptr));
        pushButton->setText(QCoreApplication::translate("LTM", "PushButton", nullptr));
    } // retranslateUi

};

namespace Ui {
    class LTM: public Ui_LTM {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LTM_H
