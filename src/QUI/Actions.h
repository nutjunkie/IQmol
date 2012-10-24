#ifndef QUI_ACTIONS_H
#define QUI_ACTIONS_H

/*!
 *  \file Actions.h 
 *
 *  \brief Declarations for custom Action functions.
 *   
 *  \author Andrew Gilbert
 *  \date August 2008
 */


#include <QWidget>
#include <QSpinBox>
#include "Logic.h"

namespace Qui {


// Widget Actions
inline Action Disable(QWidget* widget) {
   return boost::bind(&QWidget::setEnabled, widget, false);
}

inline Action Enable(QWidget* widget) {
   return boost::bind(&QWidget::setEnabled, widget, true);
}

inline Action SetValue(QSpinBox* widget, int value ) {
   return boost::bind(&QSpinBox::setValue, widget, value);
}


} // end namespace Qui
#endif
