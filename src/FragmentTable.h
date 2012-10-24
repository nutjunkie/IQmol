#ifndef IQMOL_FRAGMENTTABLE_H
#define IQMOL_FRAGMENTTABLE_H
/*******************************************************************************
       
  Copyright (C) 2011 Andrew Gilbert
           
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

#include "ui_FragmentTable.h"


namespace IQmol {

   /// A pop-up window that allows the user to select a build fragment.
   class FragmentTable : public QFrame {

      Q_OBJECT

      public: 
         FragmentTable(QWidget* parent = 0);

      private Q_SLOTS:

      private:
         void setFragmentImage(QString const& fileName);
         Ui::FragmentTable m_fragmentTable;
   };

} // end namespace IQmol

#endif
