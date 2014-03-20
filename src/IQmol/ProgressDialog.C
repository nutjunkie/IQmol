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

#include "ProgressDialog.h"
#include "Task.h"


namespace IQmol {

ProgressDialog::ProgressDialog(QString const& title, Task& task) 
{
   setWindowTitle(title); 
   setMaximum(task.totalProgress());

   connect(&task, SIGNAL(progress(int)), this, SLOT(setValue(int)));
   connect(&task, SIGNAL(finished()), this, SLOT(closeAndDelete()));
   connect(this,  SIGNAL(canceled()), &task, SLOT(stopWhatYouAreDoing()));
}


void ProgressDialog::closeAndDelete()
{
   close();
   deleteLater();
}

} // end namespace IQmol
