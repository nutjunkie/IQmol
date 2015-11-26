#ifndef IQMOL_FRAGMENTTABLE_H
#define IQMOL_FRAGMENTTABLE_H
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

#include "ui_FragmentTable.h"
#include "Viewer.h"
#include <QDir>


namespace IQmol {

   /// A pop-up window that allows the user to select a build fragment.
   class FragmentTable : public QFrame {

      Q_OBJECT

      public: 
         FragmentTable(QWidget* parent = 0);
         ~FragmentTable();

      Q_SIGNALS:
         void fragmentSelected(QString const&, Viewer::Mode const);

      private Q_SLOTS:
         void on_selectButton_clicked(bool);
         void on_efpButton_clicked(bool);
         void on_moleculesButton_clicked(bool);
         void on_functionalGroupsButton_clicked(bool);

         void selectionChanged(QTreeWidgetItem*, QTreeWidgetItem*);
         void itemDoubleClicked(QTreeWidgetItem*, int);

      private:
         static int s_fileRole;
         static int s_imageRole;
         static QString s_invalidFile;

         void setFragmentImage(QString const& fileName);
         void loadFragments();
         QList<QTreeWidgetItem*> loadFragments(QDir const& dir, QTreeWidgetItem* parent = 0);
         Ui::FragmentTable m_fragmentTable;
         QString m_fileName;
         QList<QTreeWidgetItem*> m_efp;
         QList<QTreeWidgetItem*> m_molecules;
         QList<QTreeWidgetItem*> m_functionalGroups;
         Viewer::Mode m_buildMode;
   };

} // end namespace IQmol

#endif
