#ifndef IQMOL_JOBINFO_H
#define IQMOL_JOBINFO_H
/*******************************************************************************
       
  Copyright (C) 2011-2013 Andrew Gilbert
           
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

#include <QStringList>
#include <QObject>
#include <QMap>


namespace IQmol {

   namespace Layer {
      class Molecule;
   }

   /// JobInfo forms the interface between the options editor (QUI for example)
   /// and the ProcessMonitor.
   class JobInfo : public QObject {

      Q_OBJECT

      public:
         /// BaseName       - the complete file basename, no directory, no extension
         /// InputFileName  - the input file name, no path
         /// InputString    - the entire input file as a string 
         /// RunFileName    - the name of the PBS submission script
         /// Note that not all these are serialized
         enum Field { BaseName, InputFileName, OutputFileName, AuxFileName, 
            EspFileName, MoFileName, DensityFileName, ErrorFileName,
            RunFileName, ServerName, LocalWorkingDirectory, RemoteWorkingDirectory, 
            InputString, Charge, Multiplicity, Coordinates, Constraints, EfpFragments,
            EfpParameters,
            Queue, Walltime, Memory, Scratch, Ncpus };
            
            
         JobInfo() : m_charge(0), m_multiplicity(1), m_localFilesExist(false),
           m_promptOnOverwrite(true), m_efpOnlyJob(false) { }

         /// Serialization functions that are used to reconstruct the contents of the 
         /// ProcessMonitor on restarting IQmol.  Note that it is assumed that the
         /// Process has been submitted which means we have finished with the input
         /// generator.  This obviates the need to serialize the Coordinates and 
         /// Constraints fields which are currently only requred by the input generator.
         QVariant serialize();
         static JobInfo* deserialize(QVariant const&);

         void set(Field const field, QString const& value);
         void set(Field const field, int const& value);
         QString get(Field const field) const;
         QStringList outputFiles() const;
         int getCharge() const { return m_charge; }
         int getMultiplicity() const { return m_multiplicity; }

         bool efpOnlyJob() const { return m_efpOnlyJob; }
         void setEfpOnlyJob(bool const tf) { m_efpOnlyJob = tf; }

         bool localFilesExist() const { return m_localFilesExist; }
         void localFilesExist(bool const tf) { m_localFilesExist = tf; }
         bool promptOnOverwrite() const { return m_promptOnOverwrite; }
         void promptOnOverwrite(bool const tf) { m_promptOnOverwrite = tf; }

         void dump() const;

      Q_SIGNALS:
         void updated();

      private:
         /// Generic object to hold the data
         QMap<Field,QString> m_data;
         int m_charge;
         int m_multiplicity;
         bool m_localFilesExist;
         bool m_promptOnOverwrite;
         bool m_efpOnlyJob;
   };

} // end namespace IQmol


#endif
