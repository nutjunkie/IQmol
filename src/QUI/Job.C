/*!
 *  \file Job.C 
 *
 *  \brief Non-inline member functions of the Job class, see Job.h for details.
 *   
 *  \author Andrew Gilbert
 *  \date November 2008
 */

#include "Job.h"
#include "OptionDatabase.h"
#include "Option.h"
#include "RemSection.h"
#include "MoleculeSection.h"
#include "ExternalChargesSection.h"

#include <QtDebug>  //tmp

namespace Qui {


class Molecule;

Job::Job() {
   m_remSection = new RemSection();
   m_moleculeSection = new MoleculeSection();
   m_sections["rem"] = m_remSection;
   m_sections["molecule"] = m_moleculeSection;
}


Job::Job(std::vector<KeywordSection*> sections) {
   std::vector<KeywordSection*>::iterator iter;
   for (iter = sections.begin(); iter != sections.end(); ++iter) {
       addSection(*iter);
   }
}


void Job::init() {
   MoleculeSection* molecule = m_moleculeSection->clone();
   destroy();
   addSection(molecule);
   addSection(new RemSection());
}


Job::~Job() {
   destroy();
}


void Job::destroy() {
   std::map<QString,KeywordSection*>::iterator iter;
   for (iter = m_sections.begin(); iter != m_sections.end(); ++iter) {
       delete iter->second;
   }
   m_sections.clear();
}



void Job::copy(Job const& that) {
   destroy();

   std::map<QString,KeywordSection*>::const_iterator iter;
   for (iter = that.m_sections.begin(); iter != that.m_sections.end(); ++iter) {
       addSection(iter->second->clone());
   }

   // Make sure we have the necessary sections.
   std::map<QString,KeywordSection*>::iterator iter2;

   iter2 = m_sections.find("rem");
   if (iter2 == m_sections.end()) addSection(new RemSection());

   iter2 = m_sections.find("molecule");
   if (iter2 == m_sections.end()) addSection(new MoleculeSection());

}







//! Adds the given section to the Job object, deleting any existing section of
//! the same name
void Job::addSection(KeywordSection* section) {
   QString name(section->name());
   std::map<QString,KeywordSection*>::iterator iter(m_sections.find(name));

   if (iter != m_sections.end()) {
      delete iter->second;
      m_sections.erase(iter);
   }

   m_sections.insert(std::make_pair(name,section));

   if (name == "rem") {
      m_remSection = dynamic_cast<RemSection*>(section);
   }else if (name == "molecule") {
      m_moleculeSection = dynamic_cast<MoleculeSection*>(section);
   }
}
 

StringMap Job::getOptions()  {
   StringMap opts;
   if (m_remSection) opts = m_remSection->getOptions();
   return opts;
}

QString Job::getOption(QString const& name) {
   if (m_remSection) {
      return m_remSection->getOption(name);
   }else {
      return QString();
   }
}


bool Job::isReadCoordinates() {
   if (m_moleculeSection) {
      return  m_moleculeSection->isReadCoordinates();
   }else { 
      return false;
   }
}


Molecule* Job::getMolecule() {
   if (m_moleculeSection)  {
      return m_moleculeSection->getMolecule();
   }else {
      return 0;
   }
}




void Job::setCharge(int value) {
   if (m_moleculeSection) m_moleculeSection->setCharge(value);
}

void Job::setMultiplicity(int value) {
   if (m_moleculeSection) m_moleculeSection->setMultiplicity(value);
}

void Job::setCoordinates(QString const& coords) {
   if (m_moleculeSection) m_moleculeSection->setCoordinates(coords);
}

void Job::setConstraints(QString const& constraints) {
   addSection("opt", constraints);
}


void Job::setMolecule(Molecule* mol) {
   if (m_moleculeSection) m_moleculeSection->setMolecule(mol);
}

void Job::setOption(QString const& name, QString const& value) {
   if (m_remSection) m_remSection->setOption(name, value);
}


KeywordSection* Job::addSection(QString const& name, QString const& value) {
   KeywordSection* section(KeywordSectionFactory(name));
   section->read(value);
   addSection(section);
   return section;
}


QString Job::getComment() {
   GenericSection* comment = dynamic_cast<GenericSection*>(getSection("comment"));
   return comment ? comment->rawData() : QString();
}


//! Note that this function return a null pointer if no KeywordSection of the
//! given name exists.
KeywordSection* Job::getSection(QString const& name) {
   std::map<QString,KeywordSection*>::iterator iter(m_sections.find(name));
   if (iter != m_sections.end()) {
      return m_sections[name];
   }else {
      return 0;
   }  
}


QString Job::getCoordinates() {
   if (m_moleculeSection) {
      return m_moleculeSection->getCoordinates();
   }else {
      return QString();
   }
}


int Job::getNumberOfAtoms() {
   if (m_moleculeSection) {
      return m_moleculeSection->getNumberOfAtoms();
   }else {
      return 0;
   }
}

void Job::printOption(QString const& name, bool doPrint) {
   if (m_remSection) m_remSection->printOption(name, doPrint);
}

void Job::printSection(QString const& name, bool doPrint) {
   if (doPrint && m_sections.count(name) == 0) {
      // if we should print a section then there should be one there.
      addSection(KeywordSectionFactory(name)); 
   }
   if (m_sections.count(name)) m_sections[name]->print(doPrint);
}


QString Job::format(bool const preview) {
   std::map<QString,KeywordSection*>::iterator iter, 
      begin(m_sections.begin()), end(m_sections.end());

   QString s, name;

   iter = m_sections.find("comment");
   if (iter != end) s += iter->second->format() + "\n";
   iter = m_sections.find("molecule");
   if (iter != end) s += iter->second->format() + "\n";
   iter = m_sections.find("rem");
   if (iter != end) s += iter->second->format() + "\n";


   iter = m_sections.find("external_charges");
   if (iter != end) {
      if (preview) {
         ExternalChargesSection* x;
         x = dynamic_cast<ExternalChargesSection*>(iter->second);
         s += x->previewFormat() + "\n";
      }else {
         s += iter->second->format() + "\n";
      }
   }

   for (iter = begin; iter != end; ++iter) {
	   name = iter->first;
	   if (name != "comment" && name != "molecule" && 
           name != "rem"     && name != "external_charges") {
           s += iter->second->format();
	   }
   }

   return s;
}

} // end namespace Qui
