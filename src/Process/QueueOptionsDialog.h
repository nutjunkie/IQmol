#ifndef IQMOL_PROCESS_QUEUEOPTIONSDIALOG_H
#define IQMOL_PROCESS_QUEUEOPTIONSDIALOG_H
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

#include "ui_QueueOptionsDialog.h"


namespace IQmol {
namespace Process {

   class ServerConfiguration;

   class QueueOptionsDialog : public QDialog {

      Q_OBJECT

      public:
         QueueOptionsDialog(ServerConfiguration*, QWidget* parent = 0);

      private Q_SLOTS:
         void copyToServer();

      private:
         Ui::QueueOptionsDialog m_dialog;
         ServerConfiguration* m_configuration;
   };


} } // end namespace IQmol::Process

#endif
