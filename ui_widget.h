/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QPushButton *Bt_CamClose;
    QPushButton *Bt_CamOpen;
    QPushButton *Bt_SaveImg;
    QFrame *line;
    QFrame *line_2;
    QFrame *line_3;
    QLabel *label_show;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(480, 272);
        Bt_CamClose = new QPushButton(Widget);
        Bt_CamClose->setObjectName(QStringLiteral("Bt_CamClose"));
        Bt_CamClose->setGeometry(QRect(400, 192, 80, 80));
        Bt_CamClose->setStyleSheet(QLatin1String("font: 18pt \"Sans Serif\";\n"
"background-color: rgb(255, 85, 0);"));
        Bt_CamOpen = new QPushButton(Widget);
        Bt_CamOpen->setObjectName(QStringLiteral("Bt_CamOpen"));
        Bt_CamOpen->setGeometry(QRect(400, 0, 80, 80));
        Bt_CamOpen->setStyleSheet(QLatin1String("background-color: rgb(0, 255, 0);\n"
"font: 18pt \"Sans Serif\";"));
        Bt_SaveImg = new QPushButton(Widget);
        Bt_SaveImg->setObjectName(QStringLiteral("Bt_SaveImg"));
        Bt_SaveImg->setGeometry(QRect(400, 96, 80, 80));
        Bt_SaveImg->setStyleSheet(QLatin1String("font: oblique 18pt \"Sans Serif\";\n"
"background-color: rgb(255, 255, 0);\n"
""));
        line = new QFrame(Widget);
        line->setObjectName(QStringLiteral("line"));
        line->setGeometry(QRect(400, 80, 80, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line_2 = new QFrame(Widget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setGeometry(QRect(400, 176, 80, 16));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(Widget);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setGeometry(QRect(390, 0, 10, 272));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);
        label_show = new QLabel(Widget);
        label_show->setObjectName(QStringLiteral("label_show"));
        label_show->setEnabled(true);
        label_show->setGeometry(QRect(0, 0, 390, 272));
        label_show->setStyleSheet(QLatin1String("font: oblique 16pt \"Sans Serif\";\n"
"color: rgb(85, 255, 127);"));

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Widget", 0));
        Bt_CamClose->setText(QApplication::translate("Widget", "Close", 0));
        Bt_CamOpen->setText(QApplication::translate("Widget", "Open", 0));
        Bt_SaveImg->setText(QApplication::translate("Widget", "Save", 0));
        label_show->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
