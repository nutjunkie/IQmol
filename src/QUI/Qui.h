#ifndef QUI_QUI_H
#define QUI_QUI_H

/*!
 *  \file Qui.h
 *
 *  \brief Function delclarations for non-member functions.
 *   
 *  \author Andrew Gilbert
 *  \date October 2008
 */

#include <QString>
#include <QStringList>
#include <vector>

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QRadioButton;
class QCheckBox;
class QLineEdit;
class QFile;
class QVariant;


namespace Avogadro {
   class Molecule;
}

namespace Qui {

class Job;
class KeywordSection;

void InitializeQChemLogic();
                   
void SetControl(QSpinBox*,       QString const&);
void SetControl(QCheckBox*,      QString const&);
void SetControl(QLineEdit*,      QString const&);
void SetControl(QComboBox*,      QString const&);
void SetControl(QRadioButton*,   QString const&);
void SetControl(QDoubleSpinBox*, QString const&);

// Parsing functions
void ReadInputFile(QFile& file, std::vector<Job*>*, QString* coordinates);

QString ParseXyzFileContents(QString const& lines, bool bailOnError = false);
QString ParseXyzCoordinates(QStringList const& lines, bool bailOnError = false);

QList<Job*> ParseQChemFileContents(QString const& lines);



//! Takes a string containing the contents of an input file and eats its way
//! through it, generating a vector of KeywordSections as it goes.  Note that
//! all possible sections are returned, so if there are multiple jobs then
//! there exists abiguity over which sections belong to which job.  This can be
//! resolved by spliting the string/file on @@@ before invoking this function.
std::vector<KeywordSection*> ReadKeywordSections(QString input);


#ifdef AVOGADRO
QString ExtractGeometry(Avogadro::Molecule* mol, QString const& coords);
int TotalChargeOfNuclei(Avogadro::Molecule* mol);
#endif

} // end namespace Qui

#endif
