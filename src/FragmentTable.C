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

#include "FragmentTable.h"


namespace IQmol {

FragmentTable::FragmentTable(QWidget* parent) : QFrame(parent) 
{
   m_fragmentTable.setupUi(this);
   setWindowTitle(tr("Fragment Table"));
   setFragmentImage(":/resources/fragments/water.png"); 
   QStringList solvents;
   solvents << "Acetonitrile"
 <<"Benzene"
 <<"Carbon tetrachloride"
 <<"Dichloromethane"
 <<"Dimethyl sulfoxide"
 <<"Methanol"
 <<"Phenol"
 <<"Toluene"
 <<"Water";

    //m_fragmentTable.fragmentList->addItems(solvents);


/*
Acetone_L.efp        
Cytosine_C2A_L.efp   
GuanINE_EN9RN7_L.efp 
CYTOSINE_C2B_L.efp   
GUANINE_EN9_L.efp    
THYMINE_L.efp
ADENINE_L.efp        
CYTOSINE_C3A_L.efp   
GUANINE_KN7_L.efp    
AMMONIA_L.efp        
CYTOSINE_C3B_L.efp   
GUANINE_KN9_L.efp    
METHANE_L.efp
CYTOSINE_C1_L.efp    
GUANINE_EN7_L.efp    
O2_L.efp
*/


}


void FragmentTable::setFragmentImage(QString const& fileName)
{
   QPixmap pixmap(fileName);
   QPixmap resized(pixmap.scaled(QSize(150,150)));
   QWidget* view(m_fragmentTable.viewWidget);
   QString style("background-image: url(");
   style += fileName + ");";
   view->setStyleSheet(style);
/*
   QPalette palette(view->palette());
   view->setPixmap(resized);
   //palette.setBrush(QPalette::Background, resized);
   //view->setPalette(palette);
*/
}


} // end namespace IQmol
