#ifndef IQMOL_PERIODICTABLE_H
#define IQMOL_PERIODICTABLE_H
/*******************************************************************************
       
  Copyright (C) 2011-2013 Andrew Gilbert
           
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

#include <QtGlobal>

#ifdef Q_OS_MAC
#include "ui_PeriodicTableMac.h"
#else
#include "ui_PeriodicTable.h"
#endif


namespace IQmol {

   /// A pop-up window that allows the user to select a build element.
   class PeriodicTable : public QFrame {

      Q_OBJECT

      public: 
         PeriodicTable(QWidget* parent = 0);

      Q_SIGNALS:
         void elementSelected(unsigned int atomicNumber);
         void elementSelected(QString const& atomicSymbol);

      private Q_SLOTS:
         void buttonPushed();

      private:
         void setCloseOnSelect();
         void connectSignals();
         Ui::PeriodicTable m_periodicTable;
   };

} // end namespace IQmol

#endif
