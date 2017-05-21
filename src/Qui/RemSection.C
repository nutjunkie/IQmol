/*  \file RemSection.C 
 *
 *  \brief Non-inline member functions of the RemSection class, see
 *  RemSection.h for details.
 *   
 *  \author Andrew Gilbert
 *  \date January 2008
 */

#include "RemSection.h"
#include "Option.h"
#include "OptionDatabase.h"
#include <QtDebug>
#include <QMessageBox>
#include <math.h>


namespace Qui {



//! Sets up the ad-hoc map which converts values in the QUI to values used in
//! QChem and vice versa.  The values are stored as a std::map with keys of the
//! form rem::quiValue => qchemValue and also rem::qchemValue => quiValue.  This
//! allows a given value to be mapped to different things depending on which
//! option the value is associated with.   This function is called in
//! InputDialog::initializeControl() to avoid additional database access.

QMap<QString,QString> RemSection::m_adHoc;

void RemSection::addAdHoc(QString const& rem, QString const& quiValue, 
   QString const& qchemValue) {
   // qchemValue (from the database) can be of the form qchem1|qchem2|...,
   // where all the valid QChem inputs are sparated by the | character.  For
   // the Qui->QChem mapping we have a one to one relationship, but for
   // QChem->Qui we have many to one.
   QString ucRem(rem.toUpper());
   QStringList validQChemValues(qchemValue.split("|"));

   m_adHoc[ucRem + "::" + quiValue] = validQChemValues[0];

   for (int i = 0; i < validQChemValues.count(); ++i) {
	   // Even more ad Hoc.  This avoids MP2 getting transformed to MP2[V].
	   // What really needs to be done is to have different translation units
	   // depending on which direction the translation occurs.
       if (quiValue != "MP2[V]") {
          m_adHoc[ucRem + "::" + validQChemValues[i]] = quiValue;
       }
   }
}


//! Sets the neccessary default values required for a job to run successfully.
void RemSection::init() {
   m_options.clear();
   m_toPrint.clear();

   m_options["QUI_CHARGE"] = "0";
   m_options["QUI_MULTIPLICITY"] = "1";

   m_options["METHOD"] = "HF";
   m_options["EXCHANGE"] = "HF";
   m_toPrint.insert("METHOD");

   m_options["BASIS"] = "6-31G";
   m_toPrint.insert("BASIS");

   m_options["GUI"] = "2";
   m_toPrint.insert("GUI");

   // These are necessary for obsure reasons.  Essentially this is a hack for
   // when we want to combine several controls into the one rem.  Only one of
   // them triggers the print to the input file, but the others also have to be
   // in the m_options list as they will be referenced.
/*   
   m_options["QUI_XOPT_SPIN1"]  = "Low";
   m_options["QUI_XOPT_IRREP1"] = "1";
   m_options["QUI_XOPT_STATE1"] = "0";
   m_options["QUI_XOPT_SPIN2"]  = "Low";
   m_options["QUI_XOPT_IRREP2"] = "1";
   m_options["QUI_XOPT_STATE2"] = "0";
*/
}



//! Parses a single string containing the contents of a $rem section.
void RemSection::read(QString const& input) {
   init();

   // Bit of a hack here.  The file to be read in may not have GUI set, so we
   // clear it here to avoid including it prematurely.
   printOption("GUI",false);  
   m_options["GUI"] = "0";

   QStringList lines( input.trimmed().split("\n", QString::SkipEmptyParts) );
   QStringList invalidLines;
   QStringList tokens;
   QString line;

   for (int i = 0; i < lines.count(); ++i) {
       line = lines[i].replace(QChar('='),QChar(' '));
       tokens = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);

       if (tokens.count() >= 2) {
          QString rem(tokens[0].toUpper());
          QString value(tokens[1]);
          fixOptionForQui(rem, value);
          setOption(rem, value);
          printOption(rem, true);
       }else {
          invalidLines << lines[i];
       }
   }

   if (invalidLines.count() > 0) {
      QString msg("An error occured when parsing the following options:\n");
      msg += invalidLines.join("\n");
      QMessageBox::warning(0, "Input File Error", msg); 
   }
}



QString RemSection::dump()  {
   QString s("$rem\n");
   QMap<QString,QString>::const_iterator iter;
   QString name, value;

   for (iter = m_options.begin(); iter != m_options.end(); ++iter) {
       name  = iter.key();
       value = iter.value();
       if (printOption(name) && fixOptionForQChem(name, value)) {
          s += "   " + name + "  =  " + value + "\n";
       }
   }

   s += "$end\n";
   return s;
}



RemSection* RemSection::clone() const {
   RemSection* rs = new RemSection();
   rs->setOptions(m_options);
   rs->setPrint(m_toPrint);
   return rs;
}



void RemSection::setOptions(QMap<QString,QString> const& options) {
   m_options.clear();
   m_options = options;
}



void RemSection::setPrint(std::set<QString> const& toPrint) {
   m_toPrint.clear();
   m_toPrint = toPrint;
}



QString RemSection::getOption(QString const& name) {
   QString val;
   if (m_options.count(name) > 0) {
      val = m_options[name];
   }
   return val;
}



void RemSection::printOption(QString const& option, bool print) {
   if (print) {
       m_toPrint.insert(option);
   }else {
       m_toPrint.erase(option);
   }
}



bool RemSection::fixOptionForQui(QString& name, QString& value) {
   //qDebug() << "Fixing option for QUI" << name << "=" << value;
   Option opt;
   OptionDatabase& db = OptionDatabase::instance();
   bool inDatabase(db.get(name, opt));

   // Perform some ad hoc conversions.  These are all triggered in the database
   // by having an entry of the form a//b where a is replaced by b in the input
   // file.  These are set in the InputDialog::initializeControl member function
   QString key = name + "::" + value;
   if (m_adHoc.count(key)) value = m_adHoc[key];

   //fix logicals
   if (inDatabase && opt.getType() == Option::Type_Logical) {
      if (value.toLower() == "true") {
         value = "1";
      }else if (value.toLower() == "false") {
         value = "0";
      }else if (value.toInt() != 0) {
         value = "1";
      }
   }

   // Fix CCman real variables
 /*  if (name == "CC_DIIS_MAXIMUM_OVERLAP" || 
       name == "CC_DOV_THRESH" ||
       name == "CC_DTHRESHOLD" ||
       name == "CC_HESSIAN_THRESH" ||
       name == "CC_THETA_STEPSIZE") {

       //qDebug() << "1) " << value;
       QString tmp(value);
       tmp.chop(2);
       bool ok(true);
       int exp = value.right(2).toInt(&ok);
       double man = tmp.toDouble(&ok);
       //qDebug() << "2) " << man << "x10-e" << exp;
       man *= pow(10.0,-exp);
       value = QString::number(man);
       
   }else if (inDatabase && opt.getType() == Option::Type_Real) {
      //fix other reals
      value = QString::number(value.toDouble() * opt.doubleStep());
   }*/

   if (name == "SCF_GUESS_MIX") {
      value = QString::number(value.toInt()*10);
   }

   if (name == "XC_GRID" || name == "NL_GRID") {
      bool isInt(false);
      int g(value.toInt(&isInt)); 
      if (isInt) {
         int a(g % 1000000);
         int r(g / 1000000);
         value = QString::number(a) + "," + QString::number(r); 
      }
   }

   //qDebug() << "Fixing option for QUI" << name << "=" << value;
   return true;
}


bool RemSection::fixOptionForQChem(QString& name, QString& value) {
   //qDebug() << "Fixing option for QChem" << name << "=" << value;
   bool shouldPrint(true);
   Option opt;
   OptionDatabase& db = OptionDatabase::instance();
   bool inDatabase(db.get(name, opt));

   // Skip internal QUI options
   if (name.startsWith("qui_",Qt::CaseInsensitive)) shouldPrint = false;

   if (name == "METHOD" && value == "Custom")  shouldPrint = false;

   // Perform some ad hoc conversions.  These are all triggered in the database
   // by having an entry of the form a|b where a is replaced by b in the input
   // file.  These are set in the InputDialog::initializeControl member function
   QString key = name + "::" + value;
   if (m_adHoc.count(key)) value = m_adHoc[key];

   //fix logicals
   if (inDatabase && opt.getType() == Option::Type_Logical) {
      if (name == "GUI") {
         value = value.toInt() == 0 ? QString::number(0) : QString::number(2);
      }else if (value.toInt() == Qt::Checked) {
         value = QString::number(1);
      }
   }

   //fix reals
   if (name == "CC_DIIS_MAXIMUM_OVERLAP" || 
       name == "CC_DOV_THRESH" ||
       name == "CC_DTHRESHOLD" ||
       name == "CC_HESSIAN_THRESH" ||
       name == "CC_THETA_STEPSIZE") {

       bool ok(true);
       double val = value.toDouble(&ok);

       int exp = floor(log10(val));
       int man = 100 * val * pow(10,-exp);
       //qDebug() << "2) " << man << "x10e" << exp+2;

       value = QString::number(man) + QString("%1").arg(exp+2,2,10,QChar('0'));

   }else if (inDatabase && opt.getType() == Option::Type_Real) {
      value = QString::number(value.toDouble() / opt.doubleStep());
   }


   if (name == "SCF_GUESS_MIX") {
      value = QString::number(value.toInt()/10);
   }

   if (name == "QUI_FROZEN_CORE" && value.toInt() != 0) {
      name = "N_FROZEN_CORE";
      value = "FC";
      shouldPrint = true;
   }

   if (name == "XC_GRID" || name == "NL_GRID") {
      QStringList tokens(value.split(","));
      if (tokens.size() == 2) {
         bool angularIsInt(false);
         bool radialIsInt(false);
         int  rad(tokens[0].toInt(&radialIsInt));
         int  ang(tokens[1].toInt(&angularIsInt));

         if (angularIsInt && radialIsInt) {
            value = QString("%1").arg(ang);
            value = QString::number(rad) + value.rightJustified(6,'0');
         }
      } 
   }

  
   if (name == "QUI_XOPT1" && value.toInt() > 0) {
      name = "XOPT_STATE_1";
      shouldPrint = true;

      // This is crappy
      key = "QUI_XOPT_SPIN1::";      
      key += m_options["QUI_XOPT_SPIN1"];
      if (m_adHoc.count(key)) m_options["QUI_XOPT_SPIN1"] = m_adHoc[key];

      value = "[" + m_options["QUI_XOPT_SPIN1"]  + ", "
                  + m_options["QUI_XOPT_IRREP1"] + ", "
                  + m_options["QUI_XOPT_STATE1"] + "]";

   }

   if (name == "QUI_XOPT2" && value.toInt() > 0)  {
      name = "XOPT_STATE_2";
      shouldPrint = true;

      key = "QUI_XOPT_SPIN2::";      
      key += m_options["QUI_XOPT_SPIN2"];
      if (m_adHoc.count(key)) m_options["QUI_XOPT_SPIN2"] = m_adHoc[key];

      value = "[" + m_options["QUI_XOPT_SPIN2"]  + ", "
                  + m_options["QUI_XOPT_IRREP2"] + ", "
                  + m_options["QUI_XOPT_STATE2"] + "]";
   }
 

   //qDebug() << "Fixing option for QChem" << name << "=" << value << "print = " << shouldPrint;
   return shouldPrint;
}


//! This is a debug fuction to check what ad hoc changes we are making.
void RemSection::printAdHoc() {
   QMap<QString,QString>::iterator iter;
   for (iter = m_adHoc.begin(); iter != m_adHoc.end(); ++iter) {
       qDebug() << "ADHOC::" << iter.key() << "->" << iter.value();
   }
}

} // end namespace Qui
