/********************************************************************************
** Form generated from reading UI file 'viewdlg.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIEWDLG_H
#define UI_VIEWDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_ViewDlg
{
public:
    QPushButton *Bt_Save;
    QPushButton *Bt_Measure;
    QPushButton *Bt_Cancel;
    QLabel *label_view;

    void setupUi(QDialog *ViewDlg)
    {
        if (ViewDlg->objectName().isEmpty())
            ViewDlg->setObjectName(QStringLiteral("ViewDlg"));
        ViewDlg->resize(480, 272);
        Bt_Save = new QPushButton(ViewDlg);
        Bt_Save->setObjectName(QStringLiteral("Bt_Save"));
        Bt_Save->setGeometry(QRect(120, 242, 80, 28));
        Bt_Measure = new QPushButton(ViewDlg);
        Bt_Measure->setObjectName(QStringLiteral("Bt_Measure"));
        Bt_Measure->setGeometry(QRect(220, 242, 80, 28));
        Bt_Cancel = new QPushButton(ViewDlg);
        Bt_Cancel->setObjectName(QStringLiteral("Bt_Cancel"));
        Bt_Cancel->setGeometry(QRect(320, 242, 80, 28));
        label_view = new QLabel(ViewDlg);
        label_view->setObjectName(QStringLiteral("label_view"));
        label_view->setGeometry(QRect(135, 16, 240, 210));

        retranslateUi(ViewDlg);

        QMetaObject::connectSlotsByName(ViewDlg);
    } // setupUi

    void retranslateUi(QDialog *ViewDlg)
    {
        ViewDlg->setWindowTitle(QApplication::translate("ViewDlg", "Dialog", 0));
        Bt_Save->setText(QApplication::translate("ViewDlg", "Save", 0));
        Bt_Measure->setText(QApplication::translate("ViewDlg", "Measure", 0));
        Bt_Cancel->setText(QApplication::translate("ViewDlg", "Cancel", 0));
        label_view->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class ViewDlg: public Ui_ViewDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIEWDLG_H
