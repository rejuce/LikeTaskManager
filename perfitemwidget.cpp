#include "perfitemwidget.h"
#include <QGridLayout>
#include <QBoxLayout>
#include <QPixmap>

PerfItemWidget::PerfItemWidget(const QString name,PlotType isPlotOn, QListWidget* view, bool isThreeLined) :
    QListWidgetItem(view),
    m_isPlottedOn(isPlotOn)
{
    QWidget* widget = new QWidget(view);
    QGridLayout* layout = new QGridLayout(widget);



    auto Label = new QLabel(name,widget);
    Label->setStyleSheet("background-color: rgba(191, 64, 64, 0); font-weight: bold; font-size: 22px ");
    layout->addWidget(Label, 1, 1);

    m_icon = new QLabel("",widget);
    //QPixmap tmp(":/images/cpu.png");
   // m_icon->setPixmap(tmp.scaled(QSize(22,22), Qt::KeepAspectRatio, Qt::SmoothTransformation));

    set_icon(":/images/cpu.png");
    //symbol->setScaledContents(true);
   // Label->setStyleSheet("background-color: rgba(191, 64, 64, 0); font-weight: bold; font-size: 22px ");

    layout->addWidget(m_icon, 1, 0);
    QSpacerItem *spacer0 = new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addItem(spacer0,0,1);



QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
layout->addItem(spacer,1,2);
    //layout->addWidget(new QPushButton(text), 0, 1);

    m_firstLineTxt = new QLabel(" ",widget);
   // m_firstLineTxt->setAlignment(Qt::AlignBottom);
    layout->addWidget(m_firstLineTxt, 2, 1);


    if(isThreeLined){
    m_secondLineTxt = new QLabel(" ",widget);
   // m_secondLineTxt->setAlignment(Qt::AlignTop);
    layout->addWidget(m_secondLineTxt, 3, 1);
    }

    QSpacerItem *spacer2 = new QSpacerItem(0, 5, QSizePolicy::Fixed, QSizePolicy::Fixed);
    layout->addItem(spacer2,4,1);


    widget->setLayout(layout);
     widget->setStyleSheet("background-color: transparent;}");

    this->setFlags(flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);


   // widget->setStyleSheet("background-color: rgb(173, 127, 168);");
    view->setItemWidget(this, widget);

    setSizeHint(widget->sizeHint());
}

void PerfItemWidget::set_icon(QString path)
{

    QPixmap tmp(path);
   // m_icon->setScaledContents(true);
   // m_icon->setMargin(-1);
    m_icon->setPixmap(tmp.scaled(QSize(21,21), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}
