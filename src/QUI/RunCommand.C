/*!
 *  \file RunCommand.C
 *
 *  \brief Utility function which runs a system command and returns the output.
 *
 *  \author Andrew Gilbert
 *  \date   April 2009
 */

#include <QString>
#include <QFileInfo>
#include <QProcess>
#include <QStringList>
#include <QMessageBox>

#include <QtDebug>


namespace Qui {
namespace System {

QStringList RunCommand(QFileInfo const& command, QStringList const& args, 
   unsigned int waitTimeInMilliSeconds) {
   QStringList output;

   if (command.exists()) {

      qDebug() << "Executing command" << command.absoluteFilePath() 
               << "with args:" << args;
      QProcess process;
      process.setWorkingDirectory(command.absolutePath());
      process.start(command.fileName(), args);

      if (process.waitForFinished(waitTimeInMilliSeconds)) {
         QString psOutput(process.readAllStandardOutput());
         output = psOutput.split(QRegExp("\\n"));
      }else {
         process.kill();
      }

   }else {

      QString msg("The command ");
      msg += command.absoluteFilePath();
      msg += " could not be found.";
      QMessageBox::warning(0, "File not found", msg);

   }

   return output;
}



} }  // end namespace Qui::System
