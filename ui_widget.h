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
#include <QtWidgets/QLineEdit>
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
    QPushButton *Bt_SWB;
    QFrame *line_4;
    QLineEdit *lineEdit_wbShow;
    QFrame *line_5;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QStringLiteral("Widget"));
        Widget->resize(480, 272);
        Bt_CamClose = new QPushButton(Widget);
        Bt_CamClose->setObjectName(QStringLiteral("Bt_CamClose"));
        Bt_CamClose->setGeometry(QRect(400, 210, 80, 62));
        Bt_CamClose->setStyleSheet(QLatin1String("font: 18pt \"Sans Serif\";\n"
"background-color: rgb(255, 89, 60);"));
        Bt_CamOpen = new QPushButton(Widget);
        Bt_CamOpen->setObjectName(QStringLiteral("Bt_CamOpen"));
        Bt_CamOpen->setGeometry(QRect(400, 0, 80, 62));
        Bt_CamOpen->setStyleSheet(QLatin1String("background-color: rgb(0, 255, 0);\n"
"font: 18pt \"Sans Serif\";"));
        Bt_SaveImg = new QPushButton(Widget);
        Bt_SaveImg->setObjectName(QStringLiteral("Bt_SaveImg"));
        Bt_SaveImg->setGeometry(QRect(400, 140, 80, 62));
        Bt_SaveImg->setStyleSheet(QLatin1String("font: oblique 18pt \"Sans Serif\";\n"
"background-color: rgb(255, 255, 0);\n"
""));
        line = new QFrame(Widget);
        line->setObjectName(QStringLiteral("line"));
        line->setGeometry(QRect(400, 62, 80, 8));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        line_2 = new QFrame(Widget);
        line_2->setObjectName(QStringLiteral("line_2"));
        line_2->setGeometry(QRect(400, 202, 80, 8));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);
        line_3 = new QFrame(Widget);
        line_3->setObjectName(QStringLiteral("line_3"));
        line_3->setGeometry(QRect(390, 0, 10, 272));
        line_3->setFrameShape(QFrame::VLine);
        line_3->setFrameShadow(QFrame::Sunken);
        Bt_SWB = new QPushButton(Widget);
        Bt_SWB->setObjectName(QStringLiteral("Bt_SWB"));
        Bt_SWB->setGeometry(QRect(400, 70, 80, 62));
        Bt_SWB->setStyleSheet(QLatin1String("font: oblique 18pt \"Sans Serif\";\n"
"background-color: rgb(0, 170, 127);\n"
""));
        line_4 = new QFrame(Widget);
        line_4->setObjectName(QStringLiteral("line_4"));
        line_4->setGeometry(QRect(400, 132, 80, 8));
        line_4->setFrameShape(QFrame::HLine);
        line_4->setFrameShadow(QFrame::Sunken);
        lineEdit_wbShow = new QLineEdit(Widget);
        lineEdit_wbShow->setObjectName(QStringLiteral("lineEdit_wbShow"));
        lineEdit_wbShow->setGeometry(QRect(0, 0, 390, 28));
        lineEdit_wbShow->setStyleSheet(QLatin1String("font: oblique 12pt \"Sans Serif\";\n"
"background-color: rgb(187, 255, 235);\n"
"color: rgb(29, 127, 255);"));
        line_5 = new QFrame(Widget);
        line_5->setObjectName(QStringLiteral("line_5"));
        line_5->setGeometry(QRect(0, 28, 390, 4));
        line_5->setFrameShape(QFrame::HLine);
        line_5->setFrameShadow(QFrame::Sunken);

        retranslateUi(Widget);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QApplication::translate("Widget", "Widget", 0));
        Bt_CamClose->setText(QApplication::translate("Widget", "Close", 0));
        Bt_CamOpen->setText(QApplication::translate("Widget", "Open", 0));
        Bt_SaveImg->setText(QApplication::translate("Widget", "View", 0));
        Bt_SWB->setText(QApplication::translate("Widget", "SWB", 0));
        lineEdit_wbShow->setText(QApplication::translate("Widget", "WB Mode Tip", 0));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H