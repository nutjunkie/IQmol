/*!
 *  \file KeyValueSection.C 
 *
 *  \author Andrew Gilbert
 *  \date March 2018
 */

#include "KeyValueSection.h"
#include "QsLog.h"
//#include "Qui.h"

#include <QtDebug>


namespace Qui {

KeyValueSection::KeyValueSection(QString const& name, bool print) 
  : KeywordSection(name, print)
{ 
}


KeyValueSection::~KeyValueSection() 
{
}


void KeyValueSection::printOption(QString const& key, bool print)
{
   if (print) {
      m_toPrint.insert(key);
   }else {
      m_toPrint.erase(key);
   }
}


void KeyValueSection::setOption(QString const& key, QString const& value) 
{
   m_options.insert(key,value);
}


QString KeyValueSection::dump() const
{
   QString s(m_name);
   s.prepend("$");
   s += "\n";

   QMap<QString,QString>::const_iterator iter;
   QString key, value;
   
   for (iter = m_options.begin(); iter != m_options.end(); ++iter) {
       key   = iter.key();
       if (printOption(key) ) {
          value = iter.value();
          key.replace("QUI_SOLVENT_","");
          s += "   " + key + "  " + value + "\n";
       }
   }
       
   s += "$end\n";

   return s;
}


void KeyValueSection::read(QString const& data) 
{
   QStringList lines( data.trimmed().split("\n", QString::SkipEmptyParts) );
   QStringList tokens;
   QString     line;

   for (int i = 0; i < lines.count(); ++i) {
       tokens = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

       if (tokens.count() == 1) {
          QString key(tokens[0].toUpper());
          setOption(key, "");
          printOption(key, true);
       }else if (tokens.count() >= 2) {
          QString key(tokens[0].toUpper());
          QString value(tokens[1].toUpper());
          setOption(key, value);
          printOption(key, true);
       }else {
          QLOG_DEBUG() << "Invalid line in section" << m_name << ":" << line;
       }   
   }   
}


KeyValueSection* KeyValueSection::clone() const 
{
   KeyValueSection* kvs(new KeyValueSection(m_name, m_print));
   kvs->m_options = m_options;
   kvs->m_toPrint = m_toPrint;
   return kvs;
}

} // end namespace Qui

