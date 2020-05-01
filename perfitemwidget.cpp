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
    layout->addWidget(Label, 0, 1);

    m_icon = new QLabel("",widget);
    QPixmap tmp(":/images/cpu.png");

    m_icon->setPixmap(tmp.scaled(QSize(22,22), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    //symbol->setScaledContents(true);
   // Label->setStyleSheet("background-color: rgba(191, 64, 64, 0); font-weight: bold; font-size: 22px ");
    layout->addWidget(m_icon, 0, 0);
QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
layout->addItem(spacer,0,2);
    //layout->addWidget(new QPushButton(text), 0, 1);

    m_firstLineTxt = new QLabel(" ",widget);
    layout->addWidget(m_firstLineTxt, 1, 1);

    if(isThreeLined){
    m_secondLineTxt = new QLabel(" ",widget);
    layout->addWidget(m_secondLineTxt, 2, 1);
    }

    widget->setLayout(layout);
     widget->setStyleSheet("background-color: transparent;}");

    this->setFlags(flags() | Qt::ItemIsSelectable | Qt::ItemIsEnabled);


   // widget->setStyleSheet("background-color: rgb(173, 127, 168);");
    view->setItemWidget(this, widget);

    setSizeHint(widget->sizeHint());
}
