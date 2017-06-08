#ifndef QUI_JOB_H
#define QUI_JOB_H

/*!
 *  \class Job
 *
 *  \brief A Job object represents a grouping of input sections that are
 *  required for a QChem run.  Note that there may be several Jobs contained in
 *  the one input file.  A Job must have a RemSection and a MoleculeSection,
 *  but all other sections are optional.
 *   
 *  \author Andrew Gilbert
 *  \date November 2008
 */

#include <QMap>
#include <vector>
#include <QString>



namespace Qui {

typedef QMap<QString,QString> StringMap;

class RemSection;
class KeywordSection;
class MoleculeSection;
class Molecule;

class Job {

   public:
      Job();
      ~Job();

      Job(std::vector<KeywordSection*> sections);

      Job(Job const& that) { copy(that); }

      Job const& operator=(Job const& that) {
         if (this != &that) copy (that);
         return *this;
      }
      
      QString format(bool const preview);

      void init();
      void addSection(KeywordSection* section);
      KeywordSection* addSection(QString const& name, QString const& value);

	  // Pass-through functions.  These rely on the corresponding
	  // KeywordSections to respond
      void printSection(QString const& name, bool doPrint);
      void setOption(QString const& name, QString const& value);
      void printOption(QString const& name, bool doPrint);
      void setCharge(int);
      void setMultiplicity(int);
      void setCoordinates(QString const&);
      void setCoordinatesFsm(QString const&);
      void setConstraints(QString const&);
      void setScanCoordinates(QString const&);
      void setEfpFragments(QString const&);
      void setEfpParameters(QString const&);
      void setExternalCharges(QString const&);
      void setMolecule(Molecule*);

      void setGenericSection(QString const& name, QString const& contents);

      QString  getCoordinates();
      int  getNumberOfAtoms();
      bool isReadCoordinates();

      StringMap getOptions();
      QString getOption(QString const& name);
      QString getComment();
      void setComment(QString const&);

      KeywordSection* getSection(QString const& name);
      Molecule* getMolecule();

   private:
	  //! We keep pointers to the RemSection and the Molecules section handy as
	  //! we access them frequently and do not want to have to dynamically
	  //! cast them all the time.
      RemSection* m_remSection;
      MoleculeSection* m_moleculeSection;

	  //! This contains a list of all the sections, including the RemSection
	  //! and MoleculeSection
      QMap<QString,KeywordSection*> m_sections;

	  //! destroy is responsibe for deleting any resources that we own the
	  //! pointers to
      void destroy();
      void copy(Job const& that);
};

} // end namespace Qui
#endif
