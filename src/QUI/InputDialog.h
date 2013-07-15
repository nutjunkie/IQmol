#ifndef QUI_INPUTDIALOG_H 
#define QUI_INPUTDIALOG_H 
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

#include "ui_QuiMainWindow.h"
#include "OptionRegister.h"
#include "OptionDatabase.h"
#include "Process.h"
#include <QFileInfo>
#include <QStatusBar>


class QResizeEvent;
class QFont;

namespace IQmol {
   class JobInfo;
}


namespace Qui {

class OptionDatabase;
class QtNode;
class Option;
class Molecule;
class Job;

template<class K, class T> class Register;

typedef std::map<String,String> StringMap;
typedef boost::function<void(String const&)> Update;

using Process::Monitored;


/// \brief This is the main class for the QChem input file generator.  If you
/// think this class is bloated, it is because it is.  Further member functions
/// can be found in InputDialogLogic.C and InputDialogSlots.C
class InputDialog : public QMainWindow {

   Q_OBJECT

   public:
      InputDialog(QWidget* parent = 0);
      ~InputDialog();

      /// Returns true only if the option database is loaded 
      /// correctly and the InputDialog can be used. 
      bool init();

	  /// The JobInfo object encapsulates the communication 
	  /// between the QUI and IQmol.
      void setJobInfo(IQmol::JobInfo* jobInfo);

      /// Allows the update of the servers in the server ComboBox
      void setServerList(QStringList const& servers);
      void enableSubmit(bool tf);

   public Q_SLOTS:
      void showMessage(QString const& msg);

   Q_SIGNALS:
      void submitJobRequest(IQmol::JobInfo*);


   private Q_SLOTS:
      void on_resetButton_clicked(bool) { resetJob(); }
      void on_advancedOptionsTree_itemClicked(QTreeWidgetItem* item, int col);
      void on_job_type_currentIndexChanged(QString const& text);
      void on_jobList_currentIndexChanged(int);
      void on_stackedOptions_currentChanged(int);
      void on_previewText_textChanged() { m_taint = true; }
      void on_qui_title_textChanged();
      void on_qui_charge_valueChanged(int);
      void on_qui_multiplicity_valueChanged(int);
      void on_addJobButton_clicked(bool) { appendNewJob(); }
      void on_deleteJobButton_clicked(bool); 
      void on_submitButton_clicked(bool) { submitJob(); }

      // Radio toggles for switching pages on stacked widgets
      void toggleStack(QStackedWidget* stack, bool on, QString model);
      void on_use_case_toggled(bool);
      void on_ftc_toggled(bool);
      void on_qui_cfmm_toggled(bool);
      void on_qui_solvent_cosmo_toggled(bool);
      void on_qui_solvent_pcm_toggled(bool);
      void on_qui_solvent_onsager_toggled(bool);
      void on_qui_solvent_none_toggled(bool);
      void on_smx_solvation_toggled(bool on);
      void on_svp_toggled(bool);
      void on_chemsol_toggled(bool);

      // Manual slots
      void widgetChanged(QObject* orig, QString const& value);
      void widgetChanged(QString const& value);
      void widgetChanged(int const& value);
      void widgetChanged(bool const& value);

      void changeComboBox(QString const& name, QString const& value);
      void changeCheckBox(QString const& name, QString const& value);
      void changeSpinBox(QString const& name, QString const& value);
      void changeDoubleSpinBox(QString const& name, QString const& value);
      void changeLineEdit(QString const& name, QString const& value);
      void changeRadioButton(QString const& name, QString const& value);

      void updatePreviewText();

      // Menu Slots
      void menuSave()   { saveFile(false);  }
      void menuSaveAs() { saveFile(true);  }
      void appendNewJob();
      void resetJob();
      void submitJob();
      void fontBigger()  { fontAdjust(true);  }
      void fontSmaller() { fontAdjust(false); }
      void setFont();


   protected:
      void resizeEvent(QResizeEvent* event);


   private:
      // ---------- Data ----------
      Ui::MainWindow m_ui;
      IQmol::JobInfo* m_jobInfo;
      OptionDatabase& m_db;
      OptionRegister& m_reg;
      bool m_taint;

      Job* m_currentJob;
      std::vector<Job*> m_jobs;
      std::vector<Action*> m_resetActions;
      std::map<String,Update*> m_setUpdates;
      QFileInfo m_fileIn;
      QStatusBar m_statusBar;


      // ---------- Functions ----------
      int  currentJobNumber();
      bool firstJob(Job*);
      void finalizeJob();
      bool deleteAllJobs(bool const prompt = true);

      void capturePreviewText();
      void initializeQuiLogic();
      void widgetError(QString const& name);

      void setControls(Job* job);
      void resetControls();
      void initializeMenus();
      void initializeControls();
      void initializeControl(Option const& opt, QComboBox* combo);
      void initializeControl(Option const& opt, QCheckBox* check);
      void initializeControl(Option const& opt, QLineEdit* edit);
      void initializeControl(Option const& opt, QSpinBox*  spin);
      void initializeControl(Option const& opt, QDoubleSpinBox* dspin);
      void initializeControl(Option const& opt, QRadioButton* radio);

      void connectControl(Option const& opt, QComboBox* combo);
      void connectControl(Option const& opt, QCheckBox* check);
      void connectControl(Option const& opt, QLineEdit* edit);
      void connectControl(Option const& opt, QSpinBox* spin);
      void connectControl(Option const& opt, QRadioButton* radio);
      void connectControl(Option const& opt, QDoubleSpinBox* dspin);

      void printSection(String const& name, bool doPrint);
      void updateLJParameters();
      bool saveFile(bool prompt);
      void editConstraints();

      void fontAdjust(bool makeBigger);
      void changePreviewFont(QFont const& font);
      void addJobToList(Job*);
      void appendJob(Job*);

      QString generateInputDeck(bool preview);
      QStringList generateInputDeckJobs(bool preview);
      // Deprecate
      void watchProcess(Process::Monitored* process);
      void displayJobOutput();

      QString prependRemName(QString const& name, QString const& description);
};

} // end namespace Qui

#endif
