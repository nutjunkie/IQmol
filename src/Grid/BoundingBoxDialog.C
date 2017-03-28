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

#include "BoundingBoxDialog.h"


using namespace qglviewer;

namespace IQmol {

BoundingBoxDialog::BoundingBoxDialog(qglviewer::Vec* min, qglviewer::Vec* max, 
   QWidget* parent) : QDialog(parent), m_min(min), m_max(max)
{
   m_dialog.setupUi(this);
   connect(m_dialog.buttonBox, SIGNAL(accepted()), this, SLOT(copyToInput()));
   copyFromInput();
}


void BoundingBoxDialog::copyFromInput()
{
   m_dialog.xMin->setValue(m_min->x);
   m_dialog.yMin->setValue(m_min->y);
   m_dialog.zMin->setValue(m_min->z);
   m_dialog.xMax->setValue(m_max->x);
   m_dialog.yMax->setValue(m_max->y);
   m_dialog.zMax->setValue(m_max->z);
}


void BoundingBoxDialog::copyToInput()
{
   m_min->setValue(m_dialog.xMin->value(), m_dialog.yMin->value(), m_dialog.zMin->value());
   m_max->setValue(m_dialog.xMax->value(), m_dialog.yMax->value(), m_dialog.zMax->value());
}

} // end namespace IQmol
