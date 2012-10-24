#ifndef QUI_PROCESSQCHEM_H
#define QUI_PROCESSQCHEM_H

/*!
 *  \file ProcessQChem.h 
 *
 *  \brief Class definitions associated with QChem process control in the Qui.
 *
 *  \author Andrew Gilbert
 *  \date   March 2009
 */

#include "ui_ProcessMonitor.h"
#include "Process.h"

#include <QString>
#include <QObject>
#include <QProcess>


namespace Qui {
namespace Process {


class QChem : public Monitored {

   Q_OBJECT

   public:
      QChem(QObject* parent, QString const& input, QString const& output);

      void kill();
      unsigned int exePid();

   private Q_SLOTS:
      void cleanUp(int, QProcess::ExitStatus);
      void notifyJobFinished();

   private:
      void checkForErrors();
      void renameFchkFile();
};


} } // end namespaces Qui::Process

#endif
