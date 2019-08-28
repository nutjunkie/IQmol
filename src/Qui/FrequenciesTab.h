#ifndef QUI_FREQUENCIESTAB_H
#define QUI_FREQUENCIESTAB_H

/*!
 *  \file FrequenciesTab.h 
 *  
 *  \brief contains class definitions associated with the Frequencies  tab
 *  in the toolBoxOptions widget of the QUI.
 *  
 *  \author Andrew Gilbert
 *  \date   May 2019
 */

#include "ui_FrequenciesTab.h"


namespace Qui {

class FrequenciesTab : public QWidget {

   Q_OBJECT

   public:
      FrequenciesTab(QWidget* parent) : QWidget(parent) {
         m_ui.setupUi(this);
      }

      // Public access for the logic to operate
      Ui::FrequenciesTab m_ui;
};

} // end namespaces Qui

#endif
