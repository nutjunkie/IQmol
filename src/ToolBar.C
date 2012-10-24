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

#include "ToolBar.h"
#include <QCursor>
#include <QPoint>
#include <QtDebug>


namespace IQmol {

ToolBar::ToolBar(QWidget* parent) : QWidget(parent),
   m_periodicTable(this), m_recordTimer(this)
{
   m_toolBar.setupUi(this);
   m_periodicTable.setWindowFlags(Qt::Popup);
   m_fragmentTable.setWindowFlags(Qt::Popup);
   m_recordTimer.setInterval(1000);

   connect(&m_periodicTable, SIGNAL(elementSelected(QString)), 
      this, SLOT(updateBuildElementButton(QString)));
   connect(&m_periodicTable, SIGNAL(elementSelected(unsigned int)), 
      this, SIGNAL(buildElementChanged(unsigned int)));
   connect(&m_recordTimer, SIGNAL(timeout()), 
      this, SLOT(toggleRecordButton()));
}


void ToolBar::setActiveViewerMode(ViewerMode const mode)
{
   m_toolBar.buildMode->setChecked(false);
   m_toolBar.selectMode->setChecked(false);
   m_toolBar.manipulateMode->setChecked(false);

   switch (mode) {
      case Build:      m_toolBar.buildMode->setChecked(true);      break;
      case Select:     m_toolBar.selectMode->setChecked(true);     break;
      case Manipulate: m_toolBar.manipulateMode->setChecked(true); break;
      default:  break;
   }
}


void ToolBar::updateBuildElementButton(QString symbol)
{
   m_toolBar.elementSelect->setText(symbol);
}


void ToolBar::on_buildMode_clicked(bool) 
{
   setActiveViewerMode(Build);
   changeActiveViewerMode(Build);
}


void ToolBar::on_selectMode_clicked(bool) 
{
   setActiveViewerMode(Select);
   changeActiveViewerMode(Select);
}


void ToolBar::on_manipulateMode_clicked(bool) 
{
   setActiveViewerMode(Manipulate);
   changeActiveViewerMode(Manipulate);
}


void ToolBar::on_elementSelect_clicked(bool) 
{
   QPoint pos(QCursor::pos());
   pos -= QPoint(m_periodicTable.width()/2, 25);
   m_periodicTable.move(x()+pos.x(), y()+pos.y()); 
   m_periodicTable.show(); 
}


void ToolBar::on_addFragment_clicked(bool) 
{
   QPoint pos(QCursor::pos());
   pos -= QPoint(m_fragmentTable.width()/2, 25);
   m_fragmentTable.move(x()+pos.x(), y()+pos.y()); 
   m_fragmentTable.show(); 
}


void ToolBar::setRecordAnimationButtonChecked(bool tf)
{
   m_toolBar.recordAnimation->setChecked(tf);
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
   m_toolBar.recordAnimation->setStyleSheet(styleSheet);
}


void ToolBar::recordOff()
{
   const QString styleSheet("QToolButton { "
     "background-color: QLinearGradient( x1: 0, y1: 0, x2: 0, y2: 1, "
     "stop: 0 #fff, stop: 0.5 #eee, stop: 1 #999);}");
   m_toolBar.recordAnimation->setStyleSheet(styleSheet);
}




} // end namespace IQmol
