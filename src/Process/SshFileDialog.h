#ifndef IQMOL_PROCESS_SSHFILEDIALOG_H
#define IQMOL_PROCESS_SSHFILEDIALOG_H
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

#include "ui_SshFileDialog.h"


namespace IQmol {
namespace Process {

   class ServerConfiguration;

   /// A simple dialog that allows the user to customize the locations of the
   /// SSH configuration files.
   class SshFileDialog : public QDialog {

      Q_OBJECT

      public: 
         SshFileDialog(ServerConfiguration*, QWidget* parent = 0);

      private Q_SLOTS:
         void updatePreferences();

      private:
         Ui::SshFileDialog m_dialog;
         ServerConfiguration* m_configuration;
   };

}} // end namespace IQmol::Process


#endif
