#ifndef IQMOL_PROGRESSDIALOG_H
#define IQMOL_PROGRESSDIALOG_H
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

#include <QProgressDialog>


namespace IQmol {

   class Task;

   /// Expands the QProgressDialog to interface nicely with IQmol::Tasks
  
   class ProgressDialog : public QProgressDialog {

      Q_OBJECT

      public: 
         ProgressDialog(QString const& title, Task& task);

      public Q_SLOTS:
          void closeAndDelete();

   };

} // end namespace IQmol

#endif
