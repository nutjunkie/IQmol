/*******************************************************************************
       
  Copyright (C) 2011-2015 Andrew Gilbert
           
  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.
       
  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
      
  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.  
   
********************************************************************************/

#include "PeriodicTable.h"
#include "openbabel/mol.h"
#include <QPushButton>


namespace IQmol {

PeriodicTable::PeriodicTable(QWidget* parent) : QFrame(parent) {
   m_periodicTable.setupUi(this);

   connectSignals();
   setCloseOnSelect();
   setWindowTitle(tr("Periodic Table"));
}



void PeriodicTable::connectSignals() {
   bool ok;
   QString name;
   unsigned int atomicNumber;

   QList<QPushButton*> elements(findChildren<QPushButton*>());
   QList<QPushButton*>::iterator iter;

   for (iter = elements.begin(); iter != elements.end(); ++iter) {
       connect(*iter, SIGNAL(clicked()), this, SLOT(buttonPushed()));
       name = (*iter)->objectName();
       name.remove(QRegExp("element_"));
       atomicNumber = name.toUInt(&ok);
       if (ok) {
          name = QString(OpenBabel::etab.GetName(atomicNumber).c_str());
          (*iter)->setToolTip(QString::number(atomicNumber) + " " + name);
       }
   }
}


void PeriodicTable::setCloseOnSelect() {
   QList<QPushButton*> elements(findChildren<QPushButton*>());
   QList<QPushButton*>::iterator iter;
   for (iter = elements.begin(); iter != elements.end(); ++iter) {
       connect(*iter, SIGNAL(clicked()), this, SLOT(hide()));
   }
}


void PeriodicTable::buttonPushed() {
   QPushButton* button(qobject_cast<QPushButton*>(sender()));
   QString name = button->objectName();
   name.remove(QRegExp("element_"));
   bool ok;
   int atomicNumber = name.toUInt(&ok);
   if (ok) {
      elementSelected(atomicNumber);
      elementSelected(QString(OpenBabel::etab.GetSymbol(atomicNumber)));
   }
}



} // end namespace IQmol
