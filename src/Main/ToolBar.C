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

#include "ToolBar.h"
#include <QCursor>
#include <QPoint>
#include <QtDebug>


namespace IQmol {

ToolBar::ToolBar(QWidget* parent) : QWidget(parent),
   m_periodicTable(this), m_recordTimer(this), m_buildMode(Viewer::BuildAtom)
{
   m_toolBar.setupUi(this);
   m_periodicTable.setWindowFlags(Qt::Popup);
   m_fragmentTable.setWindowFlags(Qt::Popup);
   m_recordTimer.setInterval(1000);

   connect(&m_periodicTable, SIGNAL(elementSelected(unsigned int)), 
      this, SIGNAL(buildElementSelected(unsigned int)));

   connect(&m_periodicTable, SIGNAL(elementSelected(QString const&)), 
      this, SLOT(updateBuildElement(QString const&)));

   connect(&m_fragmentTable, SIGNAL(fragmentSelected(QString const&, Viewer::Mode const)),
      this, SLOT(updateBuildFragment(QString const&, Viewer::Mode const)));

   connect(&m_recordTimer, SIGNAL(timeout()), 
      this, SLOT(toggleRecordButton()));
}


void ToolBar::setToolBarMode(Viewer::Mode const mode)
{
   // These are all the Mode-related buttons that can be toggled
   m_toolBar.manipulateButton->setChecked(false);
   m_toolBar.buildButton->setChecked(false);
   m_toolBar.elementSelectButton->setChecked(false);      
   m_toolBar.fragmentSelectButton->setChecked(false);      
   m_toolBar.selectButton->setChecked(false);

   switch (mode) {
      case Viewer::Manipulate: 
         m_toolBar.manipulateButton->setChecked(true); 
         break;

      case Viewer::Select:
         m_toolBar.selectButton->setChecked(true);     
         break;

      case Viewer::ManipulateSelection:
         m_toolBar.manipulateButton->setChecked(true); 
         break;

      case Viewer::ReindexAtoms:
         m_toolBar.manipulateButton->setChecked(true); 
         break;

      case Viewer::BuildAtom:      
         m_toolBar.buildButton->setChecked(true);      
         m_toolBar.elementSelectButton->setChecked(true);      
         m_buildMode = Viewer::BuildAtom;
         break;

      case Viewer::BuildFunctionalGroup:      
         m_toolBar.buildButton->setChecked(true);      
         m_toolBar.fragmentSelectButton->setChecked(true);      
         m_buildMode = Viewer::BuildFunctionalGroup;
         break;

      case Viewer::BuildEfp:      
         m_toolBar.buildButton->setChecked(true);      
         m_toolBar.fragmentSelectButton->setChecked(true);      
         m_buildMode = Viewer::BuildEfp;
         break;

      case Viewer::BuildMolecule:      
         m_toolBar.buildButton->setChecked(true);      
         m_toolBar.fragmentSelectButton->setChecked(true);      
         m_buildMode = Viewer::BuildMolecule;
         break;
   }
}


void ToolBar::updateBuildElement(QString const& symbol)
{
   m_buildMode = Viewer::BuildAtom;
   on_buildButton_clicked(true);
   m_toolBar.elementSelectButton->setText(symbol);
}


void ToolBar::updateBuildFragment(QString const& fileName, Viewer::Mode const mode)
{
   m_buildMode = mode;
   on_buildButton_clicked(true);
   buildFragmentSelected(fileName, mode);
}


void ToolBar::on_buildButton_clicked(bool) 
{
   setToolBarMode(m_buildMode);
   viewerModeChanged(m_buildMode);
}


void ToolBar::on_selectButton_clicked(bool) 
{
   setToolBarMode(Viewer::Select);
   viewerModeChanged(Viewer::Select);
}


void ToolBar::on_manipulateButton_clicked(bool) 
{
   setToolBarMode(Viewer::Manipulate);
   viewerModeChanged(Viewer::Manipulate);
}


void ToolBar::on_elementSelectButton_clicked(bool) 
{
   QPoint pos(QCursor::pos());
   pos -= QPoint(m_periodicTable.width()/2, 25);
   m_periodicTable.move(x()+pos.x(), y()+pos.y()); 
   m_periodicTable.show(); 
}


void ToolBar::on_fragmentSelectButton_clicked(bool) 
{
   QPoint pos(QCursor::pos());
   pos -= QPoint(m_fragmentTable.width()/2, 25);
   m_fragmentTable.move(x()+pos.x(), y()+pos.y()); 
   m_fragmentTable.show(); 
}


void ToolBar::setRecordAnimationButtonChecked(bool tf)
{
   m_toolBar.recordButton->setChecked(tf);
   if (tf) {
      recordOn();
      m_recordTimer.start();
   }else {
      m_recordTimer.stop();
      recordOff();
   }
}


void ToolBar::toggleRecordButton()
{
   static bool flash(false);
   flash = !flash;

   if (flash) {
      recordOn();
   }else {
      recordOff();
   }
}


void ToolBar::recordOn()
{
   const QString styleSheet("QToolButton { background-color: #e00; }") ;
   m_toolBar.recordButton->setStyleSheet(styleSheet);
}


void ToolBar::recordOff()
{
   const QString styleSheet("QToolButton { "
     "background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, "
     "stop: 0 #fff, stop: 0.5 #eee, stop: 1 #999);}");
   m_toolBar.recordButton->setStyleSheet(styleSheet);
}


} // end namespace IQmol
