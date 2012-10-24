/*!
 *  \file main.C 
 *  
 *  \mainpage
 *
 *  \section Introduction
 *   
 *  This is the Q-Chem user interface (QUI) designed to make the generation of input
 *  files as painless as possible.  Here you will find developer documentation
 *  extracted from the source files using Doxygen.
 *   
 *  \section Build  
 *  The QUI uses the Qt library for the graphical component.  This library has
 *  been ported to a wide variety of platforms including Windoze, OS X and
 *  Linux.  It is hoped that at least these three platforms will be able to be
 *  supported in perpetuity.
 *   
 *  In order to develop the QUI the Qt libraries need to be installed.  They
 *  can be obtained from the <a href="http://trolltech.com/">Trolltech website</a>.
 *  Qt is a mature product and the installation process is straightforward.
 *   
 *  
 *   
 *   
 *  The QUI has been designed to integrate with the Avogadro application which
 *  has also been written in Qt and is cross-platform.
 *   
 *   
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include <QApplication>
#include "OptionDatabaseForm.h"
#include "InputDialog.h"
#include <QDir>
#include <QDebug>

#include <iostream>
#include <string>


int main(int argc, char *argv[]) {

    QApplication app(argc, argv);

    QDir dir(QApplication::applicationDirPath());

#ifdef Q_WS_MAC
    dir.cdUp();
    dir.cd("Frameworks");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));

    dir.cdUp();
    dir.cd("PlugIns");
    QApplication::addLibraryPath(dir.absolutePath());
#else
    QApplication::addLibraryPath(dir.absolutePath() + "/plugins");
#endif


    //QStringList libs(QApplication::libraryPaths());
    //for (int i = 0; i< libs.count(); ++i) {
    //    std::cout << "LIBRARY: " << libs[i].toStdString() << std::endl;
    //}
 

    QString iconFile = ":/resources/icons/qchem.png";

    Q_INIT_RESOURCE(QUI);
    
    if (argc > 1 && std::string(argv[1]) == "-dbedit" ) {
       Qui::OptionDatabaseForm form;
       app.setWindowIcon(QIcon(iconFile));
       form.show();
       return app.exec();
    }else {
       Qui::InputDialog form;
       app.setWindowIcon(QIcon(iconFile));
       form.show();

       return app.exec();
    }

}
