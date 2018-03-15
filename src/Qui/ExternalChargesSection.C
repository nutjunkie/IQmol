/*!
 *  \file ExternalChargesSection.C 
 *
 *  \author Andrew Gilbert
 *  \date February 2008
 */

#include "ExternalChargesSection.h"
#include "Qui.h"

#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <QFileDialog>


namespace Qui {

void ExternalChargesSection::processData() 
{
   int numberOfCharges = m_data.count(QRegExp("\\n"));

   // This is a hack switch to undo the charge truncation
   if (numberOfCharges > 0) {
   //if (numberOfCharges < 10) {
      m_truncatedData = m_data;

   }else {
      int n(0);

      for (int i = 0; i < 5; ++i) {
         n = m_data.indexOf("\n",n+1);
      }
      m_truncatedData = m_data.left(n+1);

      m_truncatedData += "< ... +";
      m_truncatedData += QString::number(numberOfCharges-5);
      m_truncatedData += " additional charges ... >";
   }
}


QString ExternalChargesSection::dump() const
{
   QString s("$external_charges\n");
   if (!m_data.isEmpty()) s += m_data + "\n";
   s += "$end\n";
   return s;
}


void ExternalChargesSection::read(QString const& data) 
{
    m_data = data.trimmed();
    processData();
}


ExternalChargesSection* ExternalChargesSection::clone() const 
{
   return new ExternalChargesSection(m_data, m_print);
}


QString ExternalChargesSection::previewFormat() const 
{
   QString s("$external_charges\n");
   if (!m_truncatedData.isEmpty()) s += m_truncatedData + "\n";
   s += "$end\n";
   return s;
}

} // end namespace Qui

