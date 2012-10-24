/*******************************************************************************
 
  This file is part of QMsgBox
 
  QMsgBox is free software. You can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the license, or
  (at you option) any later version.
 
  QMsgBox is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with QMsgBox. If not, set <http://www.gnu.org/licenses/>.
 
  Copyleft (c) 2009 Gustavo Ribeiro Croscato
  Contact: croscato (at) gmail (dot) com
  
********************************************************************************/

#include "QMsgBox.h"
#include "QsLog.h"


QMessageBox::StandardButton QMsgBox::warning(QWidget* parent, const QString&
		title, const QString& text, QMessageBox::StandardButtons buttons,
		QMessageBox::StandardButton defaultButton)
{
	QPixmap pixmap;
    pixmap.load(":/imageWarning");

	QMessageBox messagebox(QMessageBox::NoIcon, title, text, buttons, parent);
	messagebox.setDefaultButton(defaultButton);
	messagebox.setIconPixmap(pixmap);
    QLOG_WARN() << text;

	return (QMessageBox::StandardButton) messagebox.exec();
}

QMessageBox::StandardButton QMsgBox::critical(QWidget* parent, const QString&
		title, const QString& text, QMessageBox::StandardButtons buttons,
		QMessageBox::StandardButton defaultButton)
{
	QPixmap pixmap;
    pixmap.load(":/imageCritical");

	QMessageBox messagebox(QMessageBox::NoIcon, title, text, buttons, parent);
	messagebox.setDefaultButton(defaultButton);
	messagebox.setIconPixmap(pixmap);

    QLOG_ERROR() << text;
	return (QMessageBox::StandardButton) messagebox.exec();
}

QMessageBox::StandardButton QMsgBox::information(QWidget* parent, const QString&
		title, const QString& text, QMessageBox::StandardButtons buttons,
		QMessageBox::StandardButton defaultButton)
{
	QPixmap pixmap;
    pixmap.load(":/imageInformation");

	QMessageBox messagebox(QMessageBox::NoIcon, title, text, buttons, parent);
	messagebox.setDefaultButton(defaultButton);
	messagebox.setIconPixmap(pixmap);

    QLOG_INFO() << text;
	return (QMessageBox::StandardButton) messagebox.exec();
}

QMessageBox::StandardButton QMsgBox::question(QWidget* parent, const QString&
		title, const QString& text, QMessageBox::StandardButtons buttons,
		QMessageBox::StandardButton defaultButton)
{
	QPixmap pixmap;
    pixmap.load(":/imageQuestion");

	QMessageBox messagebox(QMessageBox::NoIcon, title, text, buttons, parent);
	messagebox.setDefaultButton(defaultButton);
	messagebox.setIconPixmap(pixmap);

	return (QMessageBox::StandardButton) messagebox.exec();
}
