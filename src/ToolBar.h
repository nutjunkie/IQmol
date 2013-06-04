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
#include "Viewer.h"
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
		 /// programatically and does not emit any signals.
         void setToolBarMode(Viewer::Mode const);

      Q_SIGNALS:
         void newMolecule();
         void open();
         void save();
         void addHydrogens();
         void minimizeEnergy();
         void deleteSelection();
         void takeSnapshot();
         void record(bool const);
         void fullScreen();
         void showHelp();

		 // This signal is only emitted when the change in the Viewer::Mode
		 // originates from the tool bar.
         void viewerModeChanged(Viewer::Mode const);

         void buildElementSelected(unsigned int const atomicNumber);

         // The Viewer::Modes passed in this signal should only be build-related
         void buildFragmentSelected(QString const& fileName, Viewer::Mode const);

      public Q_SLOTS:
         void setRecordAnimationButtonChecked(bool);

      private Q_SLOTS:
         void on_newMoleculeButton_clicked(bool)      { newMolecule(); }
         void on_openButton_clicked(bool)             { open(); }
         void on_saveButton_clicked(bool)             { save(); }
         void on_manipulateButton_clicked(bool);
         void on_buildButton_clicked(bool);
         void on_elementSelectButton_clicked(bool);
         void on_fragmentSelectButton_clicked(bool);
         void on_addHydrogensButton_clicked(bool)     { addHydrogens(); }
         void on_minimizeEnergyButton_clicked(bool)   { minimizeEnergy(); }
         void on_selectButton_clicked(bool);
         void on_deleteSelectionButton_clicked(bool)  { deleteSelection(); }
         void on_takeSnapshotButton_clicked(bool)     { takeSnapshot(); }
         void on_recordButton_clicked(bool tf)        { record(tf); }
         void on_fullScreenButton_clicked(bool)       { fullScreen(); }
         void on_showHelpButton_clicked(bool)         { showHelp(); }

         void updateBuildElement(QString const&);
         void updateBuildFragment(QString const&, Viewer::Mode const);
         void toggleRecordButton();

      private:
         void recordOn();
         void recordOff();
         Ui::ToolBar   m_toolBar;
         PeriodicTable m_periodicTable;
         FragmentTable m_fragmentTable;
         QTimer m_recordTimer;
         // We use this to keep track of the last build mode used
         Viewer::Mode m_buildMode;
   };

} // end namespace IQmol

#endif
