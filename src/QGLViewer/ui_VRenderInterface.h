/********************************************************************************
** Form generated from reading UI file 'VRenderInterface.ui'
**
** Created by: Qt User Interface Compiler version 5.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VRENDERINTERFACE_H
#define UI_VRENDERINTERFACE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_VRenderInterface
{
public:
    QVBoxLayout *vboxLayout;
    QCheckBox *includeHidden;
    QCheckBox *cullBackFaces;
    QCheckBox *blackAndWhite;
    QCheckBox *colorBackground;
    QCheckBox *tightenBBox;
    QHBoxLayout *hboxLayout;
    QLabel *sortLabel;
    QComboBox *sortMethod;
    QSpacerItem *spacerItem;
    QHBoxLayout *hboxLayout1;
    QPushButton *SaveButton;
    QPushButton *CancelButton;

    void setupUi(QDialog *VRenderInterface)
    {
        if (VRenderInterface->objectName().isEmpty())
            VRenderInterface->setObjectName(QStringLiteral("VRenderInterface"));
        VRenderInterface->resize(309, 211);
        vboxLayout = new QVBoxLayout(VRenderInterface);
        vboxLayout->setSpacing(6);
        vboxLayout->setContentsMargins(11, 11, 11, 11);
        vboxLayout->setObjectName(QStringLiteral("vboxLayout"));
        vboxLayout->setContentsMargins(5, 5, 5, 5);
        includeHidden = new QCheckBox(VRenderInterface);
        includeHidden->setObjectName(QStringLiteral("includeHidden"));

        vboxLayout->addWidget(includeHidden);

        cullBackFaces = new QCheckBox(VRenderInterface);
        cullBackFaces->setObjectName(QStringLiteral("cullBackFaces"));

        vboxLayout->addWidget(cullBackFaces);

        blackAndWhite = new QCheckBox(VRenderInterface);
        blackAndWhite->setObjectName(QStringLiteral("blackAndWhite"));

        vboxLayout->addWidget(blackAndWhite);

        colorBackground = new QCheckBox(VRenderInterface);
        colorBackground->setObjectName(QStringLiteral("colorBackground"));

        vboxLayout->addWidget(colorBackground);

        tightenBBox = new QCheckBox(VRenderInterface);
        tightenBBox->setObjectName(QStringLiteral("tightenBBox"));

        vboxLayout->addWidget(tightenBBox);

        hboxLayout = new QHBoxLayout();
        hboxLayout->setSpacing(6);
        hboxLayout->setObjectName(QStringLiteral("hboxLayout"));
        hboxLayout->setContentsMargins(11, 11, 11, 11);
        sortLabel = new QLabel(VRenderInterface);
        sortLabel->setObjectName(QStringLiteral("sortLabel"));

        hboxLayout->addWidget(sortLabel);

        sortMethod = new QComboBox(VRenderInterface);
        sortMethod->setObjectName(QStringLiteral("sortMethod"));

        hboxLayout->addWidget(sortMethod);


        vboxLayout->addLayout(hboxLayout);

        spacerItem = new QSpacerItem(31, 41, QSizePolicy::Minimum, QSizePolicy::Expanding);

        vboxLayout->addItem(spacerItem);

        hboxLayout1 = new QHBoxLayout();
        hboxLayout1->setSpacing(6);
        hboxLayout1->setObjectName(QStringLiteral("hboxLayout1"));
        SaveButton = new QPushButton(VRenderInterface);
        SaveButton->setObjectName(QStringLiteral("SaveButton"));

        hboxLayout1->addWidget(SaveButton);

        CancelButton = new QPushButton(VRenderInterface);
        CancelButton->setObjectName(QStringLiteral("CancelButton"));

        hboxLayout1->addWidget(CancelButton);


        vboxLayout->addLayout(hboxLayout1);

        QWidget::setTabOrder(SaveButton, CancelButton);
        QWidget::setTabOrder(CancelButton, includeHidden);
        QWidget::setTabOrder(includeHidden, cullBackFaces);
        QWidget::setTabOrder(cullBackFaces, blackAndWhite);
        QWidget::setTabOrder(blackAndWhite, colorBackground);
        QWidget::setTabOrder(colorBackground, tightenBBox);
        QWidget::setTabOrder(tightenBBox, sortMethod);

        retranslateUi(VRenderInterface);
        QObject::connect(SaveButton, SIGNAL(released()), VRenderInterface, SLOT(accept()));
        QObject::connect(CancelButton, SIGNAL(released()), VRenderInterface, SLOT(reject()));

        sortMethod->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(VRenderInterface);
    } // setupUi

    void retranslateUi(QDialog *VRenderInterface)
    {
        VRenderInterface->setWindowTitle(QApplication::translate("VRenderInterface", "Vectorial rendering options", 0));
#ifndef QT_NO_TOOLTIP
        includeHidden->setToolTip(QApplication::translate("VRenderInterface", "Hidden polygons are also included in the output (usually twice bigger)", 0));
#endif // QT_NO_TOOLTIP
        includeHidden->setText(QApplication::translate("VRenderInterface", "Include hidden parts", 0));
#ifndef QT_NO_TOOLTIP
        cullBackFaces->setToolTip(QApplication::translate("VRenderInterface", "Back faces (non clockwise point ordering) are removed from the output", 0));
#endif // QT_NO_TOOLTIP
        cullBackFaces->setText(QApplication::translate("VRenderInterface", "Cull back faces", 0));
#ifndef QT_NO_TOOLTIP
        blackAndWhite->setToolTip(QApplication::translate("VRenderInterface", "Black and white rendering", 0));
#endif // QT_NO_TOOLTIP
        blackAndWhite->setText(QApplication::translate("VRenderInterface", "Black and white", 0));
#ifndef QT_NO_TOOLTIP
        colorBackground->setToolTip(QApplication::translate("VRenderInterface", "Use current background color instead of white", 0));
#endif // QT_NO_TOOLTIP
        colorBackground->setText(QApplication::translate("VRenderInterface", "Color background", 0));
#ifndef QT_NO_TOOLTIP
        tightenBBox->setToolTip(QApplication::translate("VRenderInterface", "Fit output bounding box to current display", 0));
#endif // QT_NO_TOOLTIP
        tightenBBox->setText(QApplication::translate("VRenderInterface", "Tighten bounding box", 0));
#ifndef QT_NO_TOOLTIP
        sortLabel->setToolTip(QApplication::translate("VRenderInterface", "Polygon depth sorting method", 0));
#endif // QT_NO_TOOLTIP
        sortLabel->setText(QApplication::translate("VRenderInterface", "Sort method:", 0));
        sortMethod->clear();
        sortMethod->insertItems(0, QStringList()
         << QApplication::translate("VRenderInterface", "No sorting", 0)
         << QApplication::translate("VRenderInterface", "BSP", 0)
         << QApplication::translate("VRenderInterface", "Topological", 0)
         << QApplication::translate("VRenderInterface", "Advanced topological", 0)
        );
#ifndef QT_NO_TOOLTIP
        sortMethod->setToolTip(QApplication::translate("VRenderInterface", "Polygon depth sorting method", 0));
#endif // QT_NO_TOOLTIP
        SaveButton->setText(QApplication::translate("VRenderInterface", "Save", 0));
        CancelButton->setText(QApplication::translate("VRenderInterface", "Cancel", 0));
    } // retranslateUi

};

namespace Ui {
    class VRenderInterface: public Ui_VRenderInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VRENDERINTERFACE_H
