/********************************************************************************
** Form generated from reading UI file 'wb_dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WB_DIALOG_H
#define UI_WB_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_wb_Dialog
{
public:
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QComboBox *comboBox;
    QRadioButton *rB_sun;
    QRadioButton *rB_cloud;
    QRadioButton *rB_rain;
    QRadioButton *rB_dusk;
    QRadioButton *rB_morn;

    void setupUi(QDialog *wb_Dialog)
    {
        if (wb_Dialog->objectName().isEmpty())
            wb_Dialog->setObjectName(QStringLiteral("wb_Dialog"));
        wb_Dialog->resize(480, 272);
        widget = new QWidget(wb_Dialog);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(10, 10, 461, 241));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        comboBox = new QComboBox(widget);
        comboBox->setObjectName(QStringLiteral("comboBox"));

        verticalLayout->addWidget(comboBox);

        rB_sun = new QRadioButton(widget);
        rB_sun->setObjectName(QStringLiteral("rB_sun"));

        verticalLayout->addWidget(rB_sun);

        rB_cloud = new QRadioButton(widget);
        rB_cloud->setObjectName(QStringLiteral("rB_cloud"));

        verticalLayout->addWidget(rB_cloud);

        rB_rain = new QRadioButton(widget);
        rB_rain->setObjectName(QStringLiteral("rB_rain"));

        verticalLayout->addWidget(rB_rain);

        rB_dusk = new QRadioButton(widget);
        rB_dusk->setObjectName(QStringLiteral("rB_dusk"));

        verticalLayout->addWidget(rB_dusk);

        rB_morn = new QRadioButton(widget);
        rB_morn->setObjectName(QStringLiteral("rB_morn"));

        verticalLayout->addWidget(rB_morn);


        retranslateUi(wb_Dialog);

        QMetaObject::connectSlotsByName(wb_Dialog);
    } // setupUi

    void retranslateUi(QDialog *wb_Dialog)
    {
        wb_Dialog->setWindowTitle(QApplication::translate("wb_Dialog", "Dialog", 0));
        rB_sun->setText(QApplication::translate("wb_Dialog", "Sun", 0));
        rB_cloud->setText(QApplication::translate("wb_Dialog", "Cloud", 0));
        rB_rain->setText(QApplication::translate("wb_Dialog", "Rain", 0));
        rB_dusk->setText(QApplication::translate("wb_Dialog", "Dusk", 0));
        rB_morn->setText(QApplication::translate("wb_Dialog", "Morn", 0));
    } // retranslateUi

};

namespace Ui {
    class wb_Dialog: public Ui_wb_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WB_DIALOG_H
