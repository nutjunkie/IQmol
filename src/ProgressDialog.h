#ifndef IQMOL_PROGRESSDIALOG_H
#define IQMOL_PROGRESSDIALOG_H
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

#include "ui_ProgressDialog.h"


namespace IQmol {

   /// A simple dialog for displaying the progress of a process.  By default
   /// the range is set to a percentage
  
   class ProgressDialog : public QDialog {

      Q_OBJECT

      public: 
         enum Flags { 
            Indefinite = 0x001,
            AllowAbort = 0x001,
         };

         ProgressDialog(QString const& title, QWidget* parent = 0);
         ~ProgressDialog() { }

         void setInfo(QString const& label);
         void setRange(int min, int max);
         void setIndefinite(bool);

      Q_SIGNALS:
         void abort();

      public Q_SLOTS:
         void updateProgress(double);  

      private Q_SLOTS:
         void on_abortButton_clicked() { abort(); }
          
      private:
         Ui::ProgressDialog m_progressDialog;
   };

} // end namespace IQmol


#endif
