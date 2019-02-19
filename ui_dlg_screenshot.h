/********************************************************************************
** Form generated from reading UI file 'dlg_screenshot.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DLG_SCREENSHOT_H
#define UI_DLG_SCREENSHOT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_Dlg_ScreenShot
{
public:
    QLabel *label_ScreenShot;

    void setupUi(QDialog *Dlg_ScreenShot)
    {
        if (Dlg_ScreenShot->objectName().isEmpty())
            Dlg_ScreenShot->setObjectName(QStringLiteral("Dlg_ScreenShot"));
        Dlg_ScreenShot->resize(320, 240);
        label_ScreenShot = new QLabel(Dlg_ScreenShot);
        label_ScreenShot->setObjectName(QStringLiteral("label_ScreenShot"));
        label_ScreenShot->setGeometry(QRect(8, 23, 291, 171));

        retranslateUi(Dlg_ScreenShot);

        QMetaObject::connectSlotsByName(Dlg_ScreenShot);
    } // setupUi

    void retranslateUi(QDialog *Dlg_ScreenShot)
    {
        Dlg_ScreenShot->setWindowTitle(QApplication::translate("Dlg_ScreenShot", "Dialog", 0));
        label_ScreenShot->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Dlg_ScreenShot: public Ui_Dlg_ScreenShot {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DLG_SCREENSHOT_H
