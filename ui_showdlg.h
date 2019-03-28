/********************************************************************************
** Form generated from reading UI file 'showdlg.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHOWDLG_H
#define UI_SHOWDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_Showdlg
{
public:
    QLabel *label_show;

    void setupUi(QDialog *Showdlg)
    {
        if (Showdlg->objectName().isEmpty())
            Showdlg->setObjectName(QStringLiteral("Showdlg"));
        Showdlg->resize(390, 240);
        label_show = new QLabel(Showdlg);
        label_show->setObjectName(QStringLiteral("label_show"));
        label_show->setGeometry(QRect(0, 0, 390, 240));

        retranslateUi(Showdlg);

        QMetaObject::connectSlotsByName(Showdlg);
    } // setupUi

    void retranslateUi(QDialog *Showdlg)
    {
        Showdlg->setWindowTitle(QApplication::translate("Showdlg", "Dialog", 0));
        label_show->setText(QApplication::translate("Showdlg", "Image Area", 0));
    } // retranslateUi

};

namespace Ui {
    class Showdlg: public Ui_Showdlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHOWDLG_H
