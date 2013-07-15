/*******************************************************************************
      
  Copyright (C) 2011 Andrew Gilbert
      
  This file is part of IQmol, a free molecular visualization program. See
  <http://iqmol.org> for more details.
   
  IQmol is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.
      
  IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.
      
  You should have received a copy of the GNU General Public License along
  with IQmol.  If not, see <http://www.gnu.org/licenses/>.
         
********************************************************************************/

#ifdef QCHEM_UI
#include "../Preferences.h"
#define Preferences IQmol::Preferences
#else
#include "Preferences.h"
#endif

#include "InputDialog.h"
#include "ExternalChargesSection.h"
#include "FileDisplay.h"
#include "GeometryConstraint.h"
#include "KeywordSection.h"
#include "Job.h"
#include "JobInfo.h"
#include "LJParametersSection.h"
#include "Option.h"
#include "OptSection.h"
#include "QtNode.h"
#include "Qui.h"
#include "QMsgBox.h"
#include "RemSection.h"
#include "ProcessQChem.h"
#include "QuiReadFile.h"
#include "QuiMolecule.h"
#include "MoleculeSection.h"

#include <QMenuBar>
#include <QClipboard>
#include <QFileDialog>
#include <QFontDialog>
#include <algorithm>
#include <QList>
#include <QApplication>
#include <QTextStream>
#include <QKeySequence>
#include <QResizeEvent>
#include <QtDebug>
#include <cstdlib>
#include <QMimeData>
#include <QFont>
#include <QPixmap>

#include <QDebug>


namespace Qui {


InputDialog::InputDialog(QWidget* parent) : 
   QMainWindow(parent), 
   m_jobInfo(0),
   m_db(OptionDatabase::instance()), 
   m_reg(OptionRegister::instance()), 
   m_taint(false), 
   m_currentJob(0), 
   m_fileIn("")
{
   m_ui.setupUi(this);
   resize(Preferences::QuiWindowSize());

   QFileInfo file(Preferences::LastFileAccessed());
   file.setFile(file.dir(),"untitled.inp");
   
   setWindowTitle("QChem Input File Editor - " + file.fileName());
   m_ui.previewText->setCurrentFont(Preferences::PreviewFont());

   setStatusBar(&m_statusBar);
}


bool InputDialog::init()
{
   if (!m_db.databaseLoaded()) {
      QString msg("QChem option database not loaded");
      QMsgBox::warning(0, "IQmol", msg);
      return false;
   }

   initializeMenus();
   initializeQuiLogic();
   initializeControls();
   appendNewJob();

   return true;
}


InputDialog::~InputDialog() {
   std::vector<Job*>::iterator iter1;
   for (iter1 = m_jobs.begin(); iter1 != m_jobs.end(); ++iter1) {
       delete *iter1;
   }
   std::vector<Action*>::iterator iter2;
   for (iter2 = m_resetActions.begin(); iter2 != m_resetActions.end(); ++iter2) {
       delete *iter2;
   }
   std::map<QString,Update*>::iterator iter3;
   for (iter3 = m_setUpdates.begin(); iter3 != m_setUpdates.end(); ++iter3) {
       delete iter3->second;
   }

}


void InputDialog::setJobInfo(IQmol::JobInfo* jobInfo)
{
   if (!jobInfo) return;
   m_jobInfo = jobInfo; 
   m_fileIn.setFile(m_jobInfo->get(IQmol::JobInfo::BaseName) + ".inp");

   m_ui.jobList->setCurrentIndex(0);
   if (!m_currentJob) {
      qDebug() << "Attempt to set JobInfo with no current Job";
      return;
   }

   m_currentJob->setCoordinates(m_jobInfo->get(IQmol::JobInfo::Coordinates));
   m_currentJob->setEfpFragments(m_jobInfo->get(IQmol::JobInfo::EfpFragments));
   m_currentJob->setConstraints(m_jobInfo->get(IQmol::JobInfo::Constraints));
   m_currentJob->setEfpParameters(m_jobInfo->get(IQmol::JobInfo::EfpParameters));


   if (m_jobInfo->efpOnlyJob()) {
      m_ui.basis->setEnabled(false);
      m_currentJob->setOption("SYMMETRY_IGNORE", "true");
      m_ui.efp_input->setEnabled(true);
      m_currentJob->setOption("EFP_INPUT", "true");
      m_currentJob->setOption("EFP_FRAGMENTS_ONLY", "true");
      m_currentJob->setOption("GUI",  "0");
   }else {
      m_ui.basis->setEnabled(true);
      m_ui.efp_input->setEnabled(false);
      m_currentJob->setOption("GUI",  "2");
      m_currentJob->setOption("EFP_FRAGMENTS_ONLY", "false");

      QString frag(m_jobInfo->get(IQmol::JobInfo::EfpFragments));
      m_ui.efp_fragments_only->setEnabled(!frag.isEmpty());
   }

   // We need the temporaries as setting the charge will overwrite the
   // jobInfo->multiplicity
   int charge(m_jobInfo->getCharge());
   int multiplicity(m_jobInfo->getMultiplicity());
   m_ui.qui_charge->setValue(charge);
   m_ui.qui_multiplicity->setValue(multiplicity);

   on_jobList_currentIndexChanged(0);
   m_taint = false;
   m_reg.get("JOB_TYPE").applyRules();

   if (m_currentJob && m_jobInfo->efpOnlyJob()) {
      m_currentJob->printOption("BASIS", false);
   }

   updatePreviewText();
}


void InputDialog::setServerList(QStringList const& servers)
{
   QComboBox* combo(m_ui.serverCombo);
   int index(combo->currentIndex());
   if (index < 0 || index > servers.size()-1) index = 0;
   combo->clear();
   combo->addItems(servers);
   combo->setCurrentIndex(index);
}


void InputDialog::enableSubmit(bool tf)
{
   m_ui.submitButton->setEnabled(tf);
}


void InputDialog::resizeEvent(QResizeEvent* event)  {
   Preferences::QuiWindowSize(event->size());
}


void InputDialog::showMessage(QString const& msg) 
{ 
   m_statusBar.showMessage(msg); 
}


/***********************************************************************
 *   
 *  Private Member functions
 *  
 ***********************************************************************/

//! The Job options are synchronised using the widgetChanged slot, but we still
//! need to determine if the options should be printed as part of the job.  This
//! is based on whether or not the associated control is enabled or not.
void InputDialog::finalizeJob() {
   if (!m_currentJob) return;

   QWidget* w;
   QString name;
   StringMap::const_iterator iter;

   StringMap s = m_currentJob->getOptions();
   for (iter = s.begin(); iter != s.end(); ++iter) {
       name  = iter->first;
       w = findChild<QWidget*>(name.toLower());
       // If there is no wiget of this name, then we are probably dealing with
       // something the user wrote into the preview box, so we just leave
       // things alone.
       if (w) m_currentJob->printOption(name, w->isEnabled());
   }
}


// ****** THE FOLLOWING needs further error checking

//! If the text has been altered by the user (as indicate by m_taint),
//! capturePreviewText takes the text from the preview panel and breaks it up
//! into blocks which are dealt with by ParseQChemFileContents
void InputDialog::capturePreviewText() {
   if (m_taint) {
      m_taint = false;
      QString text(m_ui.previewText->toPlainText());

      int currentJobIndex(0), i(0);
      std::vector<Job*>::iterator iter;
      for (iter = m_jobs.begin(); iter != m_jobs.end(); ++iter, ++i) {
          if (m_currentJob == *iter) currentJobIndex = i;
      } 

      bool prompt(false);
      deleteAllJobs(prompt);
      std::vector<Job*> jobs = ParseQChemFileContents(text);

      for (iter = jobs.begin(); iter != jobs.end(); ++iter) {
          addJobToList(*iter);
      } 

      m_ui.jobList->setCurrentIndex(currentJobIndex);
   }
}



void InputDialog::updatePreviewText() {
   bool preview(true);
   QStringList jobStrings(generateInputDeckJobs(preview));

   if ((unsigned int)jobStrings.size() != m_jobs.size()) {
      qDebug() << "ERROR: Job numbers do not match";
   }

   m_ui.previewText->clear();
   // This shouldn't really be required, but sometimes when the comment is
   // empty the default font is activated.
   m_ui.previewText->setCurrentFont(Preferences::PreviewFont());

   QString buffer;
  
   int pos(0), nJobs(m_jobs.size());
   m_ui.previewText->setTextColor("darkgrey");
   QString jobSeparator("\n@@@\n");

   for (int i = 0; i < nJobs ; ++i) {
       if (m_jobs[i] == m_currentJob) {
          pos = buffer.size();  
          m_ui.previewText->setTextColor("black");
       }
       buffer += jobStrings.value(i);
       m_ui.previewText->append(jobStrings.value(i));
       m_ui.previewText->setTextColor("darkgrey");
       if (i != nJobs-1) {
          buffer += jobSeparator;
          m_ui.previewText->append(jobSeparator);
       }
   }
 

   // This is a bit micky mouse, but I don't know of a better way of doing it.
   // ensureCursorVisible only seeks a minimal amount, so to ensure as much as
   // possible of the required section is showing, we seek to the end of the
   // text before seeking to the start of the section.
   QTextCursor cursor(m_ui.previewText->textCursor());
   cursor.setPosition(buffer.size());
   m_ui.previewText->setTextCursor(cursor);
   m_ui.previewText->ensureCursorVisible();
   cursor.setPosition(pos);
   m_ui.previewText->setTextCursor(cursor);
   m_ui.previewText->ensureCursorVisible();

   m_taint = false;
}


//! Generates the input deck based on the list of Jobs and prints this to the
//! preview text box.
QString InputDialog::generateInputDeck(bool preview) {
   return generateInputDeckJobs(preview).join("\n@@@\n\n");
}


//! Generates a list of strings containing the input for each job.
QStringList InputDialog::generateInputDeckJobs(bool preview) {
   QStringList jobStrings;

   if (m_currentJob) finalizeJob();
   capturePreviewText();

   for (unsigned int i = 0; i < m_jobs.size(); ++i) {
       jobStrings << m_jobs[i]->format(preview);
   }

   return jobStrings;
}





int InputDialog::currentJobNumber() {
   for (unsigned int i = 0; i < m_jobs.size(); ++i) {
       if (m_currentJob == m_jobs[i]) return i;
   }
   return 0;
}


bool InputDialog::firstJob(Job* job) {
   bool first(false);
   if (job && m_jobs.size() > 0) {
      first = (job == m_jobs[0]);
   }
   return first;
}


void InputDialog::printSection(String const& name, bool doPrint) {
   if (m_currentJob) m_currentJob->printSection(name, doPrint);
}


void InputDialog::updateLJParameters() {
   if (m_currentJob) {
      LJParametersSection* lj = new LJParametersSection();
      lj->generateData(m_currentJob->getCoordinates());
      m_currentJob->addSection(lj);
   }
}



// --------------- Menu Section --------------- //
void InputDialog::initializeMenus() 
{
   QMenuBar* menubar(menuBar());
   QAction* action;
   QString name;
   QMenu* menu;

   menubar->clear();

   // File
   menu = menubar->addMenu(tr("File"));

      // File -> Save As
      name = "Save As";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(menuSaveAs()));
      action->setShortcut(Qt::CTRL + Qt::Key_S );

      // File -> Close 
      name = "Close";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(close()));
      action->setShortcut(QKeySequence::Close);


   // Edit
   menu = menubar->addMenu(tr("Edit"));

      // Edit -> Copy
      name = "Copy";
      action = menu->addAction(name);

      connect(action, SIGNAL(triggered()), m_ui.previewText, SLOT(copy()));
      action->setShortcut(QKeySequence::Copy);

      // Edit -> Paste
      name = "Paste";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), m_ui.previewText, SLOT(paste()));
      action->setShortcut(QKeySequence::Paste);

      // Edit -> Cut
      name = "Cut";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), m_ui.previewText, SLOT(cut()));
      action->setShortcut(QKeySequence::Cut);


   // Job
   menu = menubar->addMenu(tr("Job"));

      // Job -> New Job Section
      name = "New Job Section";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(appendNewJob()));
      action->setShortcut(QKeySequence::New);

      // Job -> Reset
      name = "Reset";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(resetJob()));
      action->setShortcut(Qt::CTRL + Qt::Key_R );

      menu->addSeparator();

      // Job -> Submit
      name = "Submit";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(submitJob()));
      action->setShortcut(Qt::CTRL + Qt::Key_U );


   // Font
   menu = menubar->addMenu(tr("Font"));

      // Font -> Bigger
      name = "Bigger";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(fontBigger()));
      action->setShortcut(Qt::CTRL + Qt::Key_Plus);

      // Font -> Smaller 
      name = "Smaller";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(fontSmaller()));
      action->setShortcut(Qt::CTRL + Qt::Key_Minus);

      menu->addSeparator();

      // Font -> Set Font
      name = "Set Font";
      action = menu->addAction(name);
      connect(action, SIGNAL(triggered()), this, SLOT(setFont()));
      action->setShortcut(Qt::CTRL + Qt::Key_T);
}




// --------------- File Menu --------------- //

//! Saves the input file to the file specified by m_fileIn, prompting the user
//! if this is empty or if we want to over-ride the exising file name (save as).
bool InputDialog::saveFile(bool prompt) {

   QFileInfo tmp(m_fileIn);
   if (tmp.fileName().isEmpty()) {
      tmp.setFile(Preferences::LastFileAccessed());
   }
   bool saved(false);

   if (prompt) {
      tmp.setFile(QFileDialog::getSaveFileName(this, tr("Save Input File"), tmp.filePath()));
   }

   if (tmp.fileName().isEmpty()) return false;

   Preferences::LastFileAccessed(tmp.filePath());

   QFile file(tmp.filePath());
   if (file.exists() && tmp.isWritable()) file.remove();

   if (file.open(QIODevice::WriteOnly | QIODevice::Text )) {
      QByteArray buffer;
      bool preview(false);
      buffer.append(generateInputDeck(preview));
      file.write(buffer);
      file.close();
      m_fileIn = tmp;
      saved = true;
   }

   if (saved) {
      setWindowTitle("QChem Input File Editor - " + file.fileName());
   }else {
      QString msg("Could not write to file '");
      msg += tmp.fileName() + "'\nInput file not saved\n";
      QMsgBox::warning(0, "File Not Saved", msg);
   }

   return saved;
}


// --------------- Job Menu --------------- //

void InputDialog::appendNewJob() {
   appendJob(new Job());
   // The default Molecule section is set to "read", but for 
   // the first job we specify things explicitly.
   if (m_jobs.size() == 1) setJobInfo(m_jobInfo); 
}


//! Adds the specified job to the list and updates the displayed output
//! by triggering the on_jobList_currentIndexChanged(int) slot.
void InputDialog::appendJob(Job* job) {
   addJobToList(job);
   m_ui.jobList->setCurrentIndex(m_jobs.size()-1);
}


      
//! Adds the specified job to the list, but does not update the displayed
//! output.  This prevents redundant updates when adding several jobs from a file.
void InputDialog::addJobToList(Job* job) {
   if (job != NULL) {
      m_jobs.push_back(job);

      QString comment(job->getComment());
      if (comment.trimmed().isEmpty()) {
          comment = "Job " + QString::number(m_jobs.size());
      }

      m_ui.jobList->addItem(comment);
      Q_ASSERT(m_ui.jobList->count() == int(m_jobs.size()));
   }
}


void InputDialog::resetJob() {
   resetControls();
   bool prompt(false);
   deleteAllJobs(prompt);
   appendNewJob();
   updatePreviewText();
}


/********** Font *********/
//! Changes the font size either up or down by one unit.
void InputDialog::fontAdjust(bool makeBigger) {
   QFont font(Preferences::PreviewFont());
   int size = font.pointSize();
   size += makeBigger ? 1 : -1;
   font.setPointSize(size);
   changePreviewFont(font);
}


//! Prompts the user for a specific font to use in the preview box.
void InputDialog::setFont() {
   bool ok;
   QFont font(Preferences::PreviewFont());
   font = QFontDialog::getFont(&ok, font, this);
   if (ok) changePreviewFont(font);
}


//! Changes the font used to display the preview and also updates the
//! associated preference.
void InputDialog::changePreviewFont(QFont const& font) {
   Preferences::PreviewFont(font);
   if (m_taint) updatePreviewText(); // Captures any changes in the preview box
   m_ui.previewText->clear();        // This indirectly sets m_taint to true...
   m_taint = false;                  // ...so we unset it here
   m_ui.previewText->setCurrentFont(font);
   updatePreviewText();
}





 
/***********************************************************************
 *   
 *  Job Control
 *  
 ***********************************************************************/


void InputDialog::on_jobList_currentIndexChanged(int index) {
   if (0 <= index && index < int(m_jobs.size())) {
      if (m_currentJob) {
         capturePreviewText();
         finalizeJob();
      }
      m_currentJob = 0;
      resetControls();
      m_currentJob = m_jobs[index];
      setControls(m_currentJob);

      if (m_currentJob->getCoordinates().contains("read",Qt::CaseInsensitive)) {
         m_ui.qui_multiplicity->setEnabled(false);
         m_ui.qui_charge->setEnabled(false);
      }else {
         m_ui.qui_multiplicity->setEnabled(true);
         m_ui.qui_charge->setEnabled(true);
      }
      updatePreviewText();
   }
}


void InputDialog::on_deleteJobButton_clicked(bool) {
   QString msg("Are you sure you want to delete ");
   msg += m_ui.jobList->currentText() + " section?";

   if (QMsgBox::question(this, "Delete section?",msg,
       QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)  {
       return;
   }

   int i(m_ui.jobList->currentIndex());  //This is the Job we are deleting
   m_taint = false;  //This may not be right if the user has edited *other* jobs

   m_currentJob = 0;
   delete m_jobs[i];
   m_jobs.erase(m_jobs.begin()+i);
   m_ui.jobList->removeItem(i);

   if (m_jobs.size() == 0) {
      appendNewJob();
   }else {
      int j = (i == 0) ? 0 : i-1;
      m_ui.jobList->setCurrentIndex(j);
   }

   Q_ASSERT(m_ui.jobList->count() == int(m_jobs.size()));
}



//! This routine leaves the m_currentJob pointer uninitialised, and should be
//! used with caution.  Currently it is only used when capturing the text in
//! the preview box for reparsing.  The m_currentJob pointer is set imediately 
//! afterwards.
bool InputDialog::deleteAllJobs(bool const prompt) {
   if (prompt) {
      QString msg("Are you sure you want to delete all generated input?");
      if (QMessageBox::question(this, "Delete input?",msg,
          QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Cancel)  {
          return false;
      }
   }

   m_currentJob = 0;

   std::vector<Job*>::iterator iter;
   for (iter = m_jobs.begin(); iter != m_jobs.end(); ++iter) {
       delete *iter;
   }
   
   m_jobs.clear();
   m_ui.jobList->clear();
   m_ui.previewText->clear();
   m_taint = false;
   return true;
}






/***********************************************************************
 *   
 *  Process control: submitting jobs, reading output etc
 *  
 ***********************************************************************/

void InputDialog::submitJob() 
{
   bool preview(false);
   m_jobInfo->set(IQmol::JobInfo::InputString, generateInputDeck(preview));
   m_jobInfo->set(IQmol::JobInfo::ServerName, m_ui.serverCombo->currentText());
   submitJobRequest(m_jobInfo);
   return;
}


/***********************************************************************
 *   
 *  Functions for controlling specific widgets.
 *  
 *  The solvent models and large molecule options panels have stacked 
 *  widgets which control what options are visible.  These need special 
 *  treatment as appropriate slots do not exist to connect directly.
 *  
 ***********************************************************************/

// The following is used to change the displayed page on a QStackedWidget based
// on a radio box group. 
void InputDialog::toggleStack(QStackedWidget* stack, bool on, QString model) {
   QWidget* widget;
   if (on) {
      widget = stack->findChild<QWidget*>(model);
      Q_ASSERT(widget);
      widget->setEnabled(true);
      stack->setCurrentWidget(widget);
   }else {
      widget = stack->findChild<QWidget*>(model);
      if (widget) widget->setEnabled(false);
   }
}

void InputDialog::on_qui_cfmm_toggled(bool on) {
   toggleStack(m_ui.largeMoleculesStack, on, "LargeMoleculesCFMM");
}

void InputDialog::on_use_case_toggled(bool on) {
   toggleStack(m_ui.largeMoleculesStack, on, "LargeMoleculesCASE");
}

void InputDialog::on_ftc_toggled(bool on) {
   toggleStack(m_ui.largeMoleculesStack, on, "LargeMoleculesFTC");
}

void InputDialog::on_qui_solvent_pcm_toggled(bool on) {
   toggleStack(m_ui.solventStack, on, "SolventPCM");
}

void InputDialog::on_qui_solvent_cosmo_toggled(bool on) {
   toggleStack(m_ui.solventStack, on, "SolventCosmo");
}

void InputDialog::on_qui_solvent_onsager_toggled(bool on) {
   toggleStack(m_ui.solventStack, on, "SolventOnsager");
}

void InputDialog::on_qui_solvent_none_toggled(bool on) {
   toggleStack(m_ui.solventStack, on, "SolventNone");
}

void InputDialog::on_chemsol_toggled(bool on) {
   toggleStack(m_ui.solventStack, on, "SolventChemSol");
}

void InputDialog::on_smx_solvation_toggled(bool on) {
   toggleStack(m_ui.solventStack, on, "SolventSM8");
}

void InputDialog::on_svp_toggled(bool on) {
   toggleStack(m_ui.solventStack, on, "SolventSVP");
}



void InputDialog::on_advancedOptionsTree_itemClicked(QTreeWidgetItem* item, int) {
   if (item)  {
      QString label("Advanced");
      label += item->text(0).replace(" ","");
      QWidget* widget = m_ui.advancedOptionsStack->findChild<QWidget*>(label);

      if (widget) {
         m_ui.advancedOptionsStack->setCurrentWidget(widget);
      }else {
         qDebug() << "InputDialog::on_advancedOptionsTree_itemClicked:\n"
                  << "  Widget not found: " << label;
      }
   }
}


void InputDialog::on_job_type_currentIndexChanged(QString const& text) {
   QString label(text);
   if (label == "Transition State") label = "Geometry";
   label =  "Options" + label.replace(" ","");

   QWidget* widget = m_ui.stackedOptions->findChild<QWidget*>(label);
   if (widget) {
      m_ui.stackedOptions->setCurrentWidget(widget);
   }else {
     qDebug() << "InputDialog::on_job_type_currentIndexChanged:\n"
              << "  Widget not found: " << label;
   }
}


void InputDialog::on_stackedOptions_currentChanged(int index) {
    for (int i = 0; i < m_ui.stackedOptions->count(); ++i) {
        m_ui.stackedOptions->widget(i)->setEnabled(false);
    }
    m_ui.stackedOptions->widget(index)->setEnabled(true);
    updatePreviewText();
}


void InputDialog::on_qui_title_textChanged() {
   QString text(m_ui.qui_title->text());
   if (m_currentJob) {
      m_currentJob->addSection("comment", text);
      m_currentJob->printSection("comment", true);
   }

   if (text.size() > 10) {
      text.truncate(10); 
      text += "...";
   }

   int i(m_ui.jobList->currentIndex());
   m_ui.jobList->setItemText(i,text);

}


// This will only keep things consistent if the user 
// only uses the buttons to inc/dec the charge
void InputDialog::on_qui_charge_valueChanged(int value) {
   if (m_currentJob) {
      m_currentJob->setCharge(value);
      int multiplicity(m_ui.qui_multiplicity->value()); 
      multiplicity += (multiplicity == 1) ? 1 : -1;
      m_ui.qui_multiplicity->setValue(multiplicity); 
      if (m_jobInfo) {
          m_jobInfo->set(IQmol::JobInfo::Charge, value);
          m_jobInfo->set(IQmol::JobInfo::Multiplicity, multiplicity);
      }
   }
}


void InputDialog::on_qui_multiplicity_valueChanged(int value) {
   if (m_currentJob) m_currentJob->setMultiplicity(value);
   if (m_jobInfo)  m_jobInfo->set(IQmol::JobInfo::Multiplicity, value);
}




/***********************************************************************
 *   
 *  Functions for generically setting up, resetting and changing controls.
 *  
 ***********************************************************************/

// Accessing and changing the different control widgets (QComboBox, QSpinBox
// etc.) requires different member functions and so the controls cannot be
// treated polymorphically.  The following routine initializes the controls and
// (in the initializeControl subroutines) also binds Actions to be used later to
// reset and update the controls.  This allows all the controls to be treated in 
// a similar way.  What this means is that this function should be the only one
// that has the implementation case switch logic and also the only one that needs
// to perform dynamic casts on the controls.
void InputDialog::initializeControls() {

   QList<QWidget*> controls(findChildren<QWidget*>());
   QWidget* control;
   QString name, value;
   Option opt;
  
   for (int i = 0; i < controls.count(); ++i) {
       control = controls[i];
       name = control->objectName().toUpper();
 
       if (m_db.get(name, opt)) {

          switch (opt.getImplementation()) {

             case Option::Impl_None:
             break;

             case Option::Impl_Combo: {
                QComboBox* combo = qobject_cast<QComboBox*>(control);
                Q_ASSERT(combo);
                initializeControl(opt, combo);
             }
             break;

             case Option::Impl_Check: {
                QCheckBox* check = qobject_cast<QCheckBox*>(control);
                Q_ASSERT(check);
                initializeControl(opt, check);
             }
             break;
 
             case Option::Impl_Text: {
                QLineEdit* edit = qobject_cast<QLineEdit*>(control);
                Q_ASSERT(edit);
                initializeControl(opt, edit);
             }
             break;
 
             case Option::Impl_Spin: {
                QSpinBox* spin = qobject_cast<QSpinBox*>(control);
                Q_ASSERT(spin);
                initializeControl(opt, spin);
             }
             break;
 
             case Option::Impl_DSpin: {
                QDoubleSpinBox* dspin = qobject_cast<QDoubleSpinBox*>(control);
                Q_ASSERT(dspin);
                initializeControl(opt, dspin);
             }
             break;
 
             case Option::Impl_Radio: {
                QRadioButton* radio = qobject_cast<QRadioButton*>(control);
                Q_ASSERT(radio);
                initializeControl(opt, radio);
             }
             break;
 
             default: {
                qDebug() << "Error in QChem::InputDialog::initializeControl():\n"
                         << "  Could not initialize control " << name << "\n"
                         << "  Widget does not match database.  Impl:" 
                         << opt.getImplementation();
             }
             break;
          }
 
 
          if (m_reg.exists(name)) m_reg.get(name).setValue(opt.getDefaultValue());
 
       }
 
    }
}



//! A simple loop for reseting the controls to their default values.  This
//! routine takes advantage of the reset Actions that are set up in the
//! initializeControl functions.  
void InputDialog::resetControls() {
   std::vector<Action*>::iterator iter;
   for (iter = m_resetActions.begin(); iter != m_resetActions.end(); ++iter) {
       (*iter)->operator()(); 
   }
}



//! A simple loop for synchronizing controls with the list of string values
//! contained in a Job object.  This routine takes advantage of the Update
//! functions that are bind'ed in the initializeControl functions.  These Updates
//! allow a QControl widget to have its value changed based on a string,
//! irrespective of its type.
void InputDialog::setControls(Job* job) {
   StringMap::iterator iter;
   StringMap opts(job->getOptions());
   for (iter = opts.begin(); iter != opts.end(); ++iter) {
       if (m_setUpdates.count(iter->first)) {
          m_setUpdates[iter->first]->operator()(iter->second);
       }else {
          qDebug() << "Warning: Update not initialised for" 
                   << iter->first << "in InputDialog::setControls";
          qDebug() << " did you forget about it?";
       }
   }
}



//! A hack function for adding the Rem name to the tooltip documentation for
//! easier identification.  Note that the description text is in HTML, which is
//! why we have to dance around a bit.
QString InputDialog::prependRemName(QString const& name, QString const& description) {
   QString htmlName("<span style=\"font-weight:600;\">");
   htmlName += name;
   htmlName += "</span><br>";
   QString text(description);
   // This is where it gets hacky.  We need to insert the name after all the
   // html preamble which we assume is terminated by the following substring.
   // The 18 counts the number of characters past the start of "text-inp..."
   // that we need.
   int insertAt = text.indexOf("text-indent:0px;") + 18;
   return text.insert(insertAt,htmlName);
}


//! The (overloaded) initializeControl routine is responsible for the following
//! tasks:
//!  - Ensuring the control displays the appropriate options based on the
//!    contents of the OptionDatabase.  
//!  - Adding the ToolTip documentation found in the database.
//!  - Adding connections (signals & slots) between the Nodes in the
//!    OptionRegister and the control.  This allows for synchronization of the
//!    logic in InitializeQChemLogic and initializeQuiLogic.
//!  - Binding an Action to enable the control to be reset to the default value.
//!  - Binding an Update to enable the control to be reset to a string value.
void InputDialog::initializeControl(Option const& opt, QComboBox* combo) {

   QString name = opt.getName();
   QStringList opts = opt.getOptions();
   QStringList split;

   for (int i = 0; i < opts.size(); ++i) {
       // This allows for ad hoc text replacements.  This is useful so that more
       // informative text can be presented to the user which is then obfiscated
       // before being passed to QChem.  The replacements should be set in the
       // option database and have the form text//replacement.
       split = opts[i].split("//");

       if (split.size() == 1) {
          opts[i] = split[0];  
       }else if (split.size() == 2) {
          opts[i] = split[0];  
          RemSection::addAdHoc(name, split[0], split[1]);
       }else {
          qDebug() << "InputDialog::initialiseComboBox:\n"
                   << " replacement for option" << name << "is invalid:" << opts[i];
       }
   }

   combo->clear();
   combo->addItems(opts);

#if QT_VERSION >= 0x040400
   // This just allows us to add some spacers to the lists
   bool keepLooking(true);
   while (keepLooking) {
      int i = combo->findText("---", Qt::MatchStartsWith);
      if (i > 0) {
         combo->removeItem(i);
         combo->insertSeparator(i);
      }else {
         keepLooking = false;
      }
   }
#endif

   connectControl(opt, combo);
   combo->setToolTip(prependRemName(name, opt.getDescription()));

   Action* action = new Action(
      boost::bind(&QComboBox::setCurrentIndex, combo, opt.getDefaultIndex()) );
   m_resetActions.push_back(action);

   Update* update = new Update(
      boost::bind( 
         static_cast<void(*)(QComboBox*, QString const&)>(SetControl), combo, _1));
   m_setUpdates[name] = update;
}


void InputDialog::initializeControl(Option const& opt, QCheckBox* check) {
   connectControl(opt, check);
   check->setToolTip(prependRemName(opt.getName(), opt.getDescription()));

   Action* action = new Action(
      boost::bind(&QCheckBox::setChecked, check, opt.getDefaultIndex()) );
   m_resetActions.push_back(action);

   Update* update = new Update(
      boost::bind( 
         static_cast<void(*)(QCheckBox*, QString const&)>(SetControl), check, _1));
   QString name = opt.getName();
   m_setUpdates[name] = update;
}


void InputDialog::initializeControl(Option const& opt, QSpinBox* spin) {
   connectControl(opt, spin);
   spin->setToolTip(prependRemName(opt.getName(), opt.getDescription()));
   spin->setRange(opt.intMin(), opt.intMax());  
   spin->setSingleStep(opt.intStep());  

   Action* action = new Action(
      boost::bind(&QSpinBox::setValue, spin, opt.intDefault()) );
   m_resetActions.push_back(action);

   Update* update = new Update(
      boost::bind( 
         static_cast<void(*)(QSpinBox*, QString const&)>(SetControl), spin, _1));
   QString name = opt.getName();
   m_setUpdates[name] = update;
}


void InputDialog::initializeControl(Option const& opt, QDoubleSpinBox* dspin) {
   connectControl(opt, dspin);
   dspin->setToolTip(prependRemName(opt.getName(), opt.getDescription()));
   dspin->setRange(opt.doubleMin(), opt.doubleMax());  
   dspin->setSingleStep(opt.doubleStep());  

   Action* action = new Action(
      boost::bind(&QDoubleSpinBox::setValue, dspin, opt.doubleDefault()) );
   m_resetActions.push_back(action);

   Update* update = new Update(
      boost::bind( 
         static_cast<void(*)(QDoubleSpinBox*, QString const&)>(SetControl), dspin, _1));
   QString name = opt.getName();
   m_setUpdates[name] = update;
}


void InputDialog::initializeControl(Option const& opt, QRadioButton* radio) {
   connectControl(opt, radio);
   radio->setToolTip(prependRemName(opt.getName(), opt.getDescription()));

   Action* action = new Action(
      boost::bind(&QRadioButton::setChecked, radio, opt.getDefaultIndex()) );
   m_resetActions.push_back(action);

   Update* update = new Update(
      boost::bind( 
         static_cast<void(*)(QRadioButton*, QString const&)>(SetControl), radio, _1));
   QString name = opt.getName();
   m_setUpdates[name] = update;
}


void InputDialog::initializeControl(Option const& opt, QLineEdit* edit) {
   connectControl(opt, edit);
   edit->setToolTip(prependRemName(opt.getName(), opt.getDescription()));

   Action* action = new Action(
      boost::bind(&QLineEdit::setText, edit, opt.getOptionString()) );
   m_resetActions.push_back(action);

   Update* update = new Update(
      boost::bind( 
         static_cast<void(*)(QLineEdit*, QString const&)>(SetControl), edit, _1));
   QString name = opt.getName();
   m_setUpdates[name] = update;
}



//! The connectControl routines make the necessary signal-slot connections
//! between the control, the current Job and Nodes in the OptionRegister (for
//! program logic purposes).
void InputDialog::connectControl(Option const& opt, QComboBox* combo) {
   QString name = opt.getName();

   connect(combo, SIGNAL(currentIndexChanged(const QString&)),
      this, SLOT(widgetChanged(const QString&)));

   if (combo->isEditable()) {
      connect(combo, SIGNAL(editTextChanged(const QString&)),
         this, SLOT(widgetChanged(const QString&)));
   }

   if (m_reg.exists(name)) {
      connect(&(m_reg.get(name)), 
         SIGNAL(valueChanged(QString const&, QString const&)),
         this, SLOT(changeComboBox(QString const&, QString const&)) );
   }
}


void InputDialog::connectControl(Option const& opt, QRadioButton* radio) {
   QString name = opt.getName();

   connect(radio, SIGNAL(toggled(bool)), 
      this, SLOT(widgetChanged(bool)));
      
   if (m_reg.exists(name)) {
      connect(&(m_reg.get(name)), 
         SIGNAL(valueChanged(QString const&, QString const&)),
         this, SLOT(changeRadioButton(QString const&, QString const&)) );
   }
}


void InputDialog::connectControl(Option const& opt, QCheckBox* check) {
   QString name = opt.getName();

   connect(check, SIGNAL(stateChanged(int)), 
      this, SLOT(widgetChanged(int)));
      
   if (m_reg.exists(name)) {
      connect(&(m_reg.get(name)), 
         SIGNAL(valueChanged(QString const&, QString const&)),
         this, SLOT(changeCheckBox(QString const&, QString const&)) );
   }
}


void InputDialog::connectControl(Option const& opt, QDoubleSpinBox* dspin) {
   QString name = opt.getName();

   connect(dspin, SIGNAL(valueChanged(const QString&)),
      this, SLOT(widgetChanged(const QString&)));

   if (m_reg.exists(name)) {
      connect(&(m_reg.get(name)), 
         SIGNAL(valueChanged(QString const&, QString const&)),
         this, SLOT(changeDoubleSpinBox(QString const&, QString const&)) );
   }
}


void InputDialog::connectControl(Option const& opt, QSpinBox* spin) {
   QString name = opt.getName();

   connect(spin, SIGNAL(valueChanged(int)), 
      this, SLOT(widgetChanged(int)));
      
   if (m_reg.exists(name)) {
      connect(&(m_reg.get(name)), 
         SIGNAL(valueChanged(QString const&, QString const&)),
         this, SLOT(changeSpinBox(QString const&, QString const&)) );
   }
}


void InputDialog::connectControl(Option const& opt, QLineEdit* edit) {
   QString name = opt.getName();

   connect(edit, SIGNAL(textChanged(const QString&)), 
      this, SLOT(widgetChanged(const QString&)));

   if (m_reg.exists(name)) {
      connect(&(m_reg.get(name)), 
         SIGNAL(valueChanged(QString const&, QString const&)),
         this, SLOT(changeLineEdit(QString const&, QString const&)) );
   }
}



// The following are required as wrappers for the SetControl() functions
// as the signature needs to match the signals.

void InputDialog::changeComboBox(QString const& name, QString const& value) {
   QComboBox* combo = findChild<QComboBox*>(name.toLower());
   combo ? SetControl(combo, value) : widgetError(name);
}


void InputDialog::changeDoubleSpinBox(QString const& name, QString const& value) {
   QDoubleSpinBox* spin = findChild<QDoubleSpinBox*>(name.toLower());
   spin ? SetControl(spin, value) : widgetError(name);
}


void InputDialog::changeSpinBox(QString const& name, QString const& value) {
   QSpinBox* spin = findChild<QSpinBox*>(name.toLower());
   spin ? SetControl(spin, value) : widgetError(name);
}


void InputDialog::changeCheckBox(QString const& name, QString const& value) {
   QCheckBox* check = findChild<QCheckBox*>(name.toLower());
   check ? SetControl(check, value) : widgetError(name);
}


void InputDialog::changeRadioButton(QString const& name, QString const& value) {
   QRadioButton* radio = findChild<QRadioButton*>(name.toLower());
   radio ? SetControl(radio, value) : widgetError(name);
}


void InputDialog::changeLineEdit(QString const& name, QString const& value) {
   QLineEdit* edit = findChild<QLineEdit*>(name.toLower());
   edit ? SetControl(edit, value) : widgetError(name);
}


void InputDialog::widgetError(QString const& name) {
   qDebug() << "Error in QChem::InputDialog:\n"
            << "Could not find widget" << name;
}



// These widgetChanged functions are used to connect widgets to QtNodes
void InputDialog::widgetChanged(QString const& value) {
   QObject* orig = qobject_cast<QObject*>(sender());
   widgetChanged(orig, value);
}


void InputDialog::widgetChanged(int const& value) {
   QObject* orig = qobject_cast<QObject*>(sender());
   QString val(QString::number(value));
   widgetChanged(orig, val);
}

void InputDialog::widgetChanged(bool const& value) {
   QObject* orig = qobject_cast<QObject*>(sender());
   QString val = value ? QString::number(Qt::Checked) 
                       : QString::number(Qt::Unchecked);
   widgetChanged(orig, val);
}


void InputDialog::widgetChanged(QObject* orig, QString const& value) {
   QString name(orig->objectName().toUpper());
   if (m_reg.exists(name)) m_reg.get(name).setValue(value);
   if (m_currentJob) {
      m_currentJob->setOption(name, value);
      updatePreviewText();
   }
}


} // end namespace Qui
