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

#include "FragmentTable.h"
#include "Preferences.h"


namespace IQmol {

int FragmentTable::s_fileRole  = Qt::UserRole+1;
int FragmentTable::s_imageRole = Qt::UserRole+2;

QString FragmentTable::s_invalidFile = "invalidFile";

FragmentTable::FragmentTable(QWidget* parent) : QFrame(parent), m_fileName(s_invalidFile)
{
   m_fragmentTable.setupUi(this);
   setWindowTitle(tr("Fragment Table"));
   connect(m_fragmentTable.fragmentList, 
      SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this,
      SLOT(selectionChanged(QTreeWidgetItem*, QTreeWidgetItem*)));
   connect(m_fragmentTable.fragmentList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), 
      this, SLOT(itemDoubleClicked(QTreeWidgetItem*, int)));
       
   loadFragments();
   on_functionalGroupsButton_clicked(true);
}


FragmentTable::~FragmentTable()
{
   QTreeWidget* tree = m_fragmentTable.fragmentList;
   while (tree->topLevelItemCount() > 0) { tree->takeTopLevelItem(0);}
   m_efp += m_molecules;
   QList<QTreeWidgetItem*>::iterator iter;
   for (iter = m_efp.begin(); iter != m_efp.end(); ++iter) {
       delete (*iter);
   }
}


void FragmentTable::loadFragments()
{
   QDir dir(Preferences::FragmentDirectory());
   if (!dir.exists() || !dir.exists("EFP") || !dir.exists("Molecules") || 
       !dir.exists("Functional_Groups") ) {
      setFragmentImage(":resources/unhappy.png");
      m_fragmentTable.selectButton->setEnabled(false);
      return;
   }

   dir.cd("EFP");
   m_efp = loadFragments(dir);
   dir.cdUp(); 
   dir.cd("Molecules"); 
   m_molecules = loadFragments(dir);
   dir.cdUp(); 
   dir.cd("Functional_Groups"); 
   m_functionalGroups = loadFragments(dir);
}


QList<QTreeWidgetItem*> FragmentTable::loadFragments(QDir const& dir, QTreeWidgetItem* parent)
{
   QList<QTreeWidgetItem*> items;
   QTreeWidgetItem* item;
   QDir::Filters filters(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Readable);
   QStringList contents(dir.entryList(filters, QDir::DirsLast));

   QStringList::iterator iter;
   for (iter = contents.begin(); iter != contents.end(); ++iter) {
       QFileInfo info(dir, *iter);
       QString name(*iter);
       name = name.replace(".efp", " ", Qt::CaseInsensitive);
       name = name.replace(".xyz", " ", Qt::CaseInsensitive);
       name = name.replace("_L", " (L)");
       name = name.replace("_", " ");
 
       if (info.isDir()) {
          if (parent == 0) {
             item = new QTreeWidgetItem(QStringList(name));
             loadFragments(QDir(info.filePath()), item);
             items.append(item);
          }else {
             item = new QTreeWidgetItem(parent, QStringList(name));
          }
          item->setData(0, s_fileRole, s_invalidFile);
          item->setData(0, s_imageRole, s_invalidFile);

       }else if (info.suffix().contains("efp", Qt::CaseInsensitive) ||
                 info.suffix().contains("xyz", Qt::CaseInsensitive) ) {

          if (parent == 0) {
             item = new QTreeWidgetItem(QStringList(name));
             items.append(item);
          }else {
             item = new QTreeWidgetItem(parent, QStringList(name));
          }

          item->setData(0, s_fileRole, info.filePath());
          item->setData(0, s_imageRole, s_invalidFile);
          info.setFile(dir, info.completeBaseName() + ".png");

          if (info.exists()) {
              item->setData(0, s_imageRole, info.filePath());
          }else {
              qDebug() << "Image file not found:" << info.filePath();
          }
       }
   }
   return items;
}



void FragmentTable::selectionChanged(QTreeWidgetItem* current, QTreeWidgetItem*)
{
   if (current) {
      m_fileName = current->data(0, s_fileRole).toString();
      setFragmentImage(current->data(0, s_imageRole).toString());
   }
}


void FragmentTable::itemDoubleClicked(QTreeWidgetItem* item, int)
{
   m_fileName = item->data(0, s_fileRole).toString();
   on_selectButton_clicked(true);
}


void FragmentTable::on_selectButton_clicked(bool)
{
   if (m_fileName == s_invalidFile) return;
   fragmentSelected(m_fileName, m_buildMode);
   close();
}


void FragmentTable::setFragmentImage(QString const& fileName)
{
   QWidget* view(m_fragmentTable.viewWidget);
   QString style;

   if (fileName == s_invalidFile) {
      style = "background: white";
   }else {
      style = "background-image: url(";
      style += fileName + ");";
   }
   view->setStyleSheet(style);
}




void FragmentTable::on_efpButton_clicked(bool)
{
   m_buildMode = Viewer::BuildEFP;
   QTreeWidget* tree = m_fragmentTable.fragmentList;
   while (tree->topLevelItemCount() > 0) { tree->takeTopLevelItem(0);}
   tree->setColumnCount(1);
   tree->insertTopLevelItems(0, m_efp);
}


void FragmentTable::on_moleculesButton_clicked(bool)
{
   m_buildMode = Viewer::BuildMolecule;
   QTreeWidget* tree = m_fragmentTable.fragmentList;
   while (tree->topLevelItemCount() > 0) tree->takeTopLevelItem(0);
   tree->setColumnCount(1);
   tree->insertTopLevelItems(0, m_molecules);
}


void FragmentTable::on_functionalGroupsButton_clicked(bool)
{
   m_buildMode = Viewer::BuildFunctionalGroup;
   QTreeWidget* tree = m_fragmentTable.fragmentList;
   while (tree->topLevelItemCount() > 0) tree->takeTopLevelItem(0);
   tree->setColumnCount(1);
   tree->insertTopLevelItems(0, m_functionalGroups);
}
 


} // end namespace IQmol


