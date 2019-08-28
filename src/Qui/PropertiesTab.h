#ifndef QUI_PROPERTIESTAB_H
#define QUI_PROPERTIESTAB_H

/*!
 *  \file PropertiesTab.h 
 *  
 *  \brief contains class definitions associated with the Properties tab
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_PropertiesTab.h"


namespace Qui {

class PropertiesTab : public QWidget {

   Q_OBJECT

   public:
      PropertiesTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::PropertiesTab m_ui;
};

} // end namespaces Qui

#endif
