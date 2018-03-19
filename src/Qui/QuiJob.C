/*!
 *  \file Job.C 
 *
 *  \brief Non-inline member functions of the Job class, see Job.h for details.
 *   
 *  \author Andrew Gilbert
 *  \date November 2008
 */

#include "QuiJob.h"
#include "OptionDatabase.h"
#include "Option.h"
#include "QuiMolecule.h"
#include "RemSection.h"
#include "MoleculeSection.h"
#include "KeyValueSection.h"
#include "ExternalChargesSection.h"

#include <QDebug>

namespace Qui {



Job::Job() {
   m_remSection      = new RemSection();
   m_moleculeSection = new MoleculeSection();
   m_solventSection  = new KeyValueSection("solvent",false);
   m_pcmSection      = new KeyValueSection("pcm",false);
   m_smxSection      = new KeyValueSection("smx",false);
   m_chemsolSection  = new KeyValueSection("chemsol",false);

   m_sections["rem"]      = m_remSection;
   m_sections["molecule"] = m_moleculeSection;
   m_sections["solvent"]  = m_solventSection;
   m_sections["pcm"]      = m_pcmSection;
   m_sections["smx"]      = m_smxSection;
   m_sections["chemsol"]  = m_chemsolSection;
}


Job::Job(std::vector<KeywordSection*> sections) {
   std::vector<KeywordSection*>::iterator iter;
   for (iter = sections.begin(); iter != sections.end(); ++iter) {
       addSection(*iter);
   }
}


Job::~Job() {
   destroy();
}


void Job::destroy() {
   QMap<QString,KeywordSection*>::iterator iter;
   for (iter = m_sections.begin(); iter != m_sections.end(); ++iter) {
       delete iter.value();
   }
   m_sections.clear();
}


void Job::copy(Job const& that) {
   destroy();

   QMap<QString,KeywordSection*>::const_iterator iter;
   for (iter = that.m_sections.begin(); iter != that.m_sections.end(); ++iter) {
       addSection(iter.value()->clone());
   }

   // Make sure we have the necessary sections.
   QMap<QString,KeywordSection*>::iterator iter2;

   iter2 = m_sections.find("rem");
   if (iter2 == m_sections.end()) addSection(new RemSection());

   iter2 = m_sections.find("molecule");
   if (iter2 == m_sections.end()) addSection(new MoleculeSection());

}





//! Adds the given section to the Job object, deleting any existing section of
//! the same name
void Job::addSection(KeywordSection* section) {
   QString name(section->name());
   QMap<QString,KeywordSection*>::iterator iter(m_sections.find(name));

   if (iter != m_sections.end()) {
      delete iter.value();
      m_sections.erase(iter);
   }

   m_sections.insert(name, section);

   if (name == "rem") {
      m_remSection = dynamic_cast<RemSection*>(section);
   }else if (name == "molecule") {
      m_moleculeSection = dynamic_cast<MoleculeSection*>(section);
   }else if (name == "solvent") {
      m_solventSection = dynamic_cast<KeyValueSection*>(section);
   }else if (name == "pcm") {
      m_pcmSection = dynamic_cast<KeyValueSection*>(section);
   }else if (name == "smx") {
      m_smxSection = dynamic_cast<KeyValueSection*>(section);
   }else if (name == "chemsol") {
      m_chemsolSection = dynamic_cast<KeyValueSection*>(section);
   }
}
 

StringMap Job::getOptions()  
{
   StringMap opts;
   if (m_remSection)     opts  = m_remSection->getOptions();
   if (m_solventSection) opts.unite(m_solventSection->getOptions());
   if (m_pcmSection)     opts.unite(m_pcmSection->getOptions());
   if (m_smxSection)     opts.unite(m_smxSection->getOptions());
   if (m_chemsolSection) opts.unite(m_chemsolSection->getOptions());
   return opts;
}


QString Job::getOption(QString const& name) 
{
   if (m_remSection) {
      return m_remSection->getOption(name);
   }else {
      return QString();
   }
}


bool Job::isReadCoordinates() 
{
   if (m_moleculeSection) {
      return  m_moleculeSection->isReadCoordinates();
   }else { 
      return false;
   }
}


Molecule* Job::getMolecule() 
{
   if (m_moleculeSection)  {
      return m_moleculeSection->getMolecule();
   }else {
      return 0;
   }
}




void Job::setCharge(int value) 
{
   if (m_moleculeSection) m_moleculeSection->setCharge(value);
}


void Job::setMultiplicity(int value) 
{
   if (m_moleculeSection) m_moleculeSection->setMultiplicity(value);
}


void Job::setCoordinates(QString const& coords) 
{
   if (m_moleculeSection) {
      m_moleculeSection->setCoordinates(coords);
   }
}


void Job::setCoordinatesFsm(QString const& coords) 
{
   if (m_moleculeSection) m_moleculeSection->setCoordinatesFsm(coords);
}


void Job::setConstraints(QString const& constraints) 
{
   addSection("opt", constraints);
}


void Job::setScanCoordinates(QString const& scan) 
{
   addSection("scan", scan);
}



void Job::setEfpFragments(QString const& efpFragments) 
{
   KeywordSection* efp = addSection("efp_fragments", efpFragments);
   efp->print(!efpFragments.isEmpty());
}


void Job::setEfpParameters(QString const& efpParameters) 
{
   KeywordSection* efp = addSection("efp_parameters", efpParameters);
   efp->print(!efpParameters.isEmpty());
}


void Job::setExternalCharges(QString const& charges) 
{
   if (charges.isEmpty()) return;
   ExternalChargesSection* externalCharges(new ExternalChargesSection(charges));
   addSection(externalCharges);
   externalCharges->print(!charges.isEmpty());
}


void Job::setGenericSection(QString const& name, QString const& contents)
{
   KeywordSection* section = addSection(name, contents);
   section->print(false);
}


void Job::setMolecule(Molecule* mol) 
{
   if (m_moleculeSection) {
      m_moleculeSection->setMolecule(mol);
   }
}


void Job::setOption(QString const& name, QString const& value) 
{
   // Note these prefixes need to be stripped in KeyValueSection::setOption
   if (name.startsWith("QUI_SOLVENT_", Qt::CaseInsensitive)) {
      if (m_solventSection) m_solventSection->setOption(name, value);

   }else if (name.startsWith("QUI_PCM_", Qt::CaseInsensitive)) {
      if (m_pcmSection) m_pcmSection->setOption(name, value);

   }else if (name.startsWith("QUI_SMX_", Qt::CaseInsensitive)) {
      if (m_smxSection) m_smxSection->setOption(name, value);

   }else if (name.startsWith("QUI_CHEMSOL_", Qt::CaseInsensitive)) {
      if (m_chemsolSection) m_chemsolSection->setOption(name, value);

   }else if (m_remSection) {
      m_remSection->setOption(name, value);
      if (name.toUpper() == "JOB_TYPE" && m_moleculeSection) {
         m_moleculeSection->setFsm(value.toLower().contains("freezing string"));
      }
   }
}


KeywordSection* Job::addSection(QString const& name, QString const& value) 
{
   KeywordSection* section(KeywordSectionFactory(name));
   section->read(value);
   addSection(section);
   return section;
}


QString Job::getComment() 
{
   GenericSection* comment = dynamic_cast<GenericSection*>(getSection("comment"));
   return comment ? comment->rawData() : QString();
}


void Job::setComment(QString const& s) 
{
   GenericSection* comment = dynamic_cast<GenericSection*>(getSection("comment"));
   if (comment) {
      comment->read(s);
   }else {
      addSection("comment", s);
   }
   setOption("QUI_TITLE", s);
}



//! Note that this function return a null pointer if no KeywordSection of the
//! given name exists.
KeywordSection* Job::getSection(QString const& name) {
   QMap<QString,KeywordSection*>::iterator iter(m_sections.find(name));
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


void Job::printOption(QString const& name, bool doPrint) 
{
   if (name.startsWith("QUI_SOLVENT")) {
      if (m_solventSection) m_solventSection->printOption(name, doPrint);
   }else if (name.startsWith("QUI_PCM")) {
      if (m_pcmSection) m_pcmSection->printOption(name, doPrint);
   }else {
      if (m_remSection) m_remSection->printOption(name, doPrint);
   }
}


void Job::printSection(QString const& name, bool doPrint) {
   if (doPrint && m_sections.count(name) == 0) {
      // if we should print a section then there should be one there.
      addSection(KeywordSectionFactory(name)); 
   }
   if (m_sections.count(name)) m_sections[name]->print(doPrint);
}


QString Job::format(bool const preview) 
{
   QMap<QString,KeywordSection*>::iterator iter, 
      begin(m_sections.begin()), end(m_sections.end());

   QString s, name;

   iter = m_sections.find("comment");
   if (iter != end) s += iter.value()->format();
   iter = m_sections.find("molecule");
   if (iter != end) s += iter.value()->format();
   iter = m_sections.find("rem");
   if (iter != end) s += iter.value()->format();


   iter = m_sections.find("external_charges");
   if (iter != end) {
      if (preview) {
         ExternalChargesSection* x;
         x = dynamic_cast<ExternalChargesSection*>(iter.value());
         s += x->previewFormat();
      }else {
         s += iter.value()->format();
      }
   }

   for (iter = begin; iter != end; ++iter) {
	   name = iter.key();
	   if (name != "comment" && name != "molecule" && 
           name != "rem"     && name != "external_charges") {
           s += iter.value()->format();
	   }
   }

   return s;
}

} // end namespace Qui
