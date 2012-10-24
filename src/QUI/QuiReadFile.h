#ifndef QUI_READFILE_H
#define QUI_READFILE_H

/*!
 *  \file ReadFile.h
 *
 *  \brief Non-member utility functions for quickly reading the contents of text files.
 *
 *  \author Andrew Gilbert
 *  \date   September 2010
 */
 
#include <QFile>
#include <QFileInfo>

QStringList ReadFileToList(QFile& file);

QString ReadFileToString(QFile& file);

QStringList ReadFileToList(QFileInfo const& info);

QString ReadFileToString(QFileInfo const& info);

#endif
