/*!
 *  \file ReadFile.C
 *
 *  \brief Non-member utility functions for reading the contents of text files.
 *   
 *  \author Andrew Gilbert
 *  \date   September 2010
 */


#include "QuiReadFile.h"
#include <QMessageBox>
#include <QTextStream>


//! This is the main routine that does the work
QStringList ReadFileToList(QFile& file) {
   QStringList contents;

   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream in(&file);
      QString line;

      while (!in.atEnd()) {
         line = in.readLine();
         line = line.trimmed();
         contents << line;
      }
      file.close();
   }else {
      QString msg("An error occured when trying to read the file");
      msg += file.fileName() + "\n";
      msg += "Either the file does not exist, is unreadable or is not a valid text file.";
      QMessageBox::warning(0, "I/O Error", msg);
   }

   return contents;
}



QString ReadFileToString(QFile& file) {
   return ReadFileToList(file).join("\n");
}



QStringList ReadFileToList(QFileInfo const& info) {
   QStringList contents;
   QString msg("");

   if (!info.exists()) {
      msg = "File does not exist";
   }else if (!info.isReadable()) {
      msg = "File is not readable";
   }else {
      QFile file(info.filePath());
      contents = ReadFileToList(file);
   } 

   if (!msg.isEmpty()) {
      QString error("An error occured when trying to read the file\n");
      error += info.filePath() + "\n";
      error += msg;
      QMessageBox::warning(0, "I/O Error", error);
   }

   return contents;
}



QString ReadFileToString(QFileInfo const& info) {
   return ReadFileToList(info).join("\n");
}
