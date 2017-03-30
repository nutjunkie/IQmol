/*!
 *  \file PcmSection.C 
 *
 *  \author Andrew Gilbert
 *  \date February 2008
 */

#include "PcmSection.h"
#include "Qui.h"

#include <QFile>
#include <QtDebug>
#include <QFileInfo>
#include <QFileDialog>


namespace Qui {

void PcmSection::processData() 
{
  // do nothing for now
}


QString PcmSection::dump() {
   QString s("$pcm\n");
   if (!m_data.isEmpty()) s += m_data + "\n";
   s += "$end\n";
   return s;
}


void PcmSection::read(QString const& data) {
    m_data = data.trimmed();
    processData();
}


PcmSection* PcmSection::clone() const {
   return new PcmSection(m_data, m_print);
}


QString PcmSection::previewFormat() const {
   QString s("$pcm\n");
   if (!m_data.isEmpty()) s += m_data + "\n";
   s += "$end\n";
   return s;
}

} // end namespace Qui

