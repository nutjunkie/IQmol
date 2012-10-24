#ifndef QUI_FILEGLOB_H
#define QUI_FILEGLOB_H

/*!
 *  \class FileGlob
 *
 *  \brief Globs together the i/o files for a QChem job.  This class also 
 *   
 *  \author Andrew Gilbert
 *  \date May 2008
 */

#include <QString>
#include <QStringList>


namespace Qui {


class FileGlob {

   public:
      QString inputFileName()  const { return m_infile;  }
      QString outputFileName() const { return m_outfile; }
      QString tmpFileName();   const { return m_tmpfile; }
      QString auxFileName();   const { return m_auxfile; }

   private:
      QString m_infile;
      QString m_outfile;
      QString m_tmpfile;
      QString m_auxfile;
};

} // end namespace Qui
#endif
