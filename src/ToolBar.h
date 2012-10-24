#ifndef IQMOL_TOOLBAR_H
#define IQMOL_TOOLBAR_H
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

#include "ui_ToolBar.h"
#include "PeriodicTable.h"
#include "FragmentTable.h"
#include "HelpBrowser.h"
#include "IQmol.h"
#include <QTimer>


namespace IQmol {

   /// The ToolBar is provides a widget with several QToolButtons that trigger
   /// common behaviour including setting the Viewer mode and build element.
   /// The ToolBar communicates with the Viewer and ViewerModel via signals.
   class ToolBar : public QWidget {

      Q_OBJECT

      public: 
         ToolBar(QWidget* parent = 0);
         ~ToolBar() { }

      public Q_SLOTS:
		 /// This is used to update the appearance of the ToolBar
		 /// programatically.  It does not relay any signals.
         void setActiveViewerMode(ViewerMode const);

      Q_SIGNALS:
         void newMolecule();
         void open();
         void save();
         void changeActiveViewerMode(ViewerMode const);
         void buildElementChanged(unsigned int atomicNumber);
         void addHydrogens();
         void addFragment();
         void minimizeEnergy();
         void deleteSelection();
         void takeSnapshot();
         void recordingActive(bool);
         void fullScreen();
         void showHelp();

      public Q_SLOTS:
         void setRecordAnimationButtonChecked(bool);

      private Q_SLOTS:
         void on_newMolecule_clicked(bool)        { newMolecule(); }
         void on_open_clicked(bool)               { open(); }
         void on_save_clicked(bool)               { save(); }
         void on_manipulateMode_clicked(bool);
         void on_buildMode_clicked(bool);
         void on_elementSelect_clicked(bool);
         void on_addHydrogens_clicked(bool)       { addHydrogens(); }
         void on_addFragment_clicked(bool);
         //void on_addFragment_clicked(bool)     { addFragment(); }
         void on_minimizeEnergy_clicked(bool)     { minimizeEnergy(); }
         void on_selectMode_clicked(bool);
         void on_deleteSelection_clicked(bool)    { deleteSelection(); }
         void on_takeSnapshot_clicked(bool)       { takeSnapshot(); }
         void on_recordAnimation_clicked(bool tf) { recordingActive(tf); }
         void on_fullScreen_clicked(bool)         { fullScreen(); }
         void on_showHelp_clicked(bool)           { showHelp(); }
         void updateBuildElementButton(QString);
         void toggleRecordButton();

      private:
         void recordOn();
         void recordOff();
         Ui::ToolBar   m_toolBar;
         PeriodicTable m_periodicTable;
         FragmentTable m_fragmentTable;
         QTimer m_recordTimer;
   };

} // end namespace IQmol

#endif
