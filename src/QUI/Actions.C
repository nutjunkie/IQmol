/*!  
 *  \file Conditions.C
 *   
 *  \brief Definitions of the Actions used in the QChem Logic.
 *   
 *  \author Andrew Gilbert
 *  \date August 2008
 */


#include "Actions.h"
#include "OptionRegister.h"

#include <QToolBox>
#include <QDebug>


namespace Qui {


void AddToolBoxPage(QToolBox* toolBox, QWidget* page, QString const& pageName)
{
   if (!page) {
      qDebug() << "Null page pointer passed for" << pageName;
      return;
   }

   int index(toolBox->indexOf(page));
   if (index < 0) {
      toolBox->addItem(page, pageName);
      qDebug() << "Page added to QToolBox" << pageName;
   }
   page->show();
   page->setEnabled(true);
   toolBox->setCurrentWidget(page);
}


void RemoveToolBoxPages(QToolBox* toolBox, QStringList const& pageNames)
{
   int nPages(toolBox->count());  

   // Step backwards as we are removing pages from the tool box
   for (int i = nPages-1; i >= 0; --i) {
       if (pageNames.contains(toolBox->itemText(i))) {
          qDebug() << "Page removed from QToolBox" << toolBox->itemText(i);
          toolBox->widget(i)->hide();
          toolBox->widget(i)->setEnabled(false);
          toolBox->removeItem(i);
       } 
   }
}

} // end namespace Qui
