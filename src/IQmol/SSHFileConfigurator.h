#ifndef IQMOL_SSHFILECONFIGURATOR_H
#define IQMOL_SSHFILECONFIGURATOR_H
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

#include "ui_SSHFileConfigurator.h"


namespace IQmol {

   /// A simple dialog that allows the user to customize the locations of the
   /// SSH configuration files.
   class SSHFileConfigurator : public QDialog {

      Q_OBJECT

      public: 
         SSHFileConfigurator(QWidget* parent = 0);

      private Q_SLOTS:
         void updatePreferences();

      private:
         Ui::SSHFileConfigurator m_dialog;
   };

} // end namespace IQmol


#endif
