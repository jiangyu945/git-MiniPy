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
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>

QT_BEGIN_NAMESPACE

class Ui_ShowDlg
{
public:
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *ShowDlg)
    {
        if (ShowDlg->objectName().isEmpty())
            ShowDlg->setObjectName(QStringLiteral("ShowDlg"));
        ShowDlg->resize(480, 272);
        buttonBox = new QDialogButtonBox(ShowDlg);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(400, 0, 80, 272));
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        retranslateUi(ShowDlg);
        QObject::connect(buttonBox, SIGNAL(accepted()), ShowDlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), ShowDlg, SLOT(reject()));

        QMetaObject::connectSlotsByName(ShowDlg);
    } // setupUi

    void retranslateUi(QDialog *ShowDlg)
    {
        ShowDlg->setWindowTitle(QApplication::translate("ShowDlg", "Dialog", 0));
    } // retranslateUi

};

namespace Ui {
    class ShowDlg: public Ui_ShowDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHOWDLG_H
