/*!
 *  \file Qui.C
 *
 *  \brief Miscellaneous non-member functions
 *
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "Qui.h"

#include <QString>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLineEdit>


namespace Qui {

void SetControl(QComboBox* combo, QString const& value) {
   int i = combo->findText(value, Qt::MatchFixedString);
   if (i >= 0) combo->setCurrentIndex(i);
}

void SetControl(QDoubleSpinBox* spin, QString const& value) {
   spin->setValue(value.toDouble());
}

void SetControl(QSpinBox* spin, QString const& value) {
   spin->setValue(value.toInt());
}

void SetControl(QRadioButton* radio, QString const& value) {
   radio->setChecked(value.toInt());
}

void SetControl(QLineEdit* edit, QString const& value) {
   edit->setText(value);
}

void SetControl(QCheckBox* check, QString const& value) {
   if (value.toLower() == "true") {
      check->setChecked(true);
   }else if (value.toLower() == "false") {
      check->setChecked(false);
   }else {
      check->setChecked(value.toInt());
   }
}

} // end namespace Qui
