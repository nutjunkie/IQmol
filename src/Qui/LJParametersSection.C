/*!
 *  \file LJParametersSection.C 
 *
 *  \author Andrew Gilbert
 *  \date March 2008
 */

#include "LJParametersSection.h"
#include <QMessageBox>
#include <QRegExp>

#include <QtDebug>


namespace Qui {


std::map<QString,QString> 
LJParametersSection::s_parameters = LJParametersSection::createMap();


std::map<QString,QString> LJParametersSection::createMap() 
{
   std::map<QString,QString> parameters;
   parameters["C"]  = "  0.00010  7.60";
   parameters["H"]  = "  0.00005  4.20";
   parameters["O"]  = "  0.00030  6.30";
   parameters["N"]  = "  0.00020  7.40";
   parameters["S"]  = "  0.00040  8.50";
   parameters["F"]  = "  0.00114  5.20";
   parameters["P"]  = "  0.00030  9.10";
   parameters["CL"] = "  0.00018  8.40";
   parameters["BR"] = "  0.00050  9.00";
   parameters["I"]  = "  0.00065  9.50";

   return parameters;
}


void LJParametersSection::read(QString const& input) 
{
   m_data = input;
}


void LJParametersSection::generateData(QString const& geometry) 
{
   QStringList tokens;
   QStringList lines(geometry.split(QRegExp("\\n")));
   std::map<QString, QString>::iterator iter, end(s_parameters.end());
   m_data.clear();

   QString atom;
   QString notFound;

   for (int i = 0; i < lines.size(); ++i) {
       tokens = lines[i].split( QRegExp("\\s+"), QString::SkipEmptyParts);
       if (tokens.count() > 1) {
          atom = tokens[0];
          iter = s_parameters.find(atom.toUpper());
          m_data += QString::number(i+1);
          if (iter == end) {
             notFound += " " + atom;
          }else {
             m_data += iter->second;
          }
          m_data += "\n";
       }
   }

   if (!notFound.isEmpty()) {
      QString msg("The molecule contains atoms for which there are no inbuilt "
                  "Lennard-Jones parameters:");
      msg += notFound;
      QMessageBox::warning(0, "LJ Parameter Error",msg);
  
   }
 
}



QString  LJParametersSection::dump() const
{
   QString s("$lj_parameters\n");
   s += m_data;
   s += "$end\n";
   return s;
}


LJParametersSection* LJParametersSection::clone() const 
{
   return new  LJParametersSection(m_data);
}


} // end namespace Qui
