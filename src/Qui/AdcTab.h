#ifndef QUI_ADCTAB_H
#define QUI_ADCTAB_H

/*!
 *  \file AdcTab.h 
 *  
 *  \brief contains class definitions associated with the ADC tab
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_AdcTab.h"


namespace Qui {

class AdcTab : public QWidget {

   Q_OBJECT

   public:
     AdcTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
     }

     // Public access for the logic to operate
     Ui::AdcTab m_ui;
};

} // end namespaces Qui

#endif
