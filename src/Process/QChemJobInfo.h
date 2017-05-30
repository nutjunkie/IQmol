#ifndef IQMOL_PROCESS_QCHEMJOBINFO_H
#define IQMOL_PROCESS_QCHEMJOBINFO_H
/*******************************************************************************
       
  Copyright (C) 2011-2015 Andrew Gilbert
           
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
#include <QMap>
#include "JobInfo.h"


namespace IQmol {
namespace Process {

   /// QChemJobInfo holds information about the job to be run and forms the
   /// interface between the options editor (QUI) and the JobMonitor.
   class QChemJobInfo : public JobInfo {

      public:
         /// InputFileName  - the input file name, no path
         /// InputString    - the entire input file as a string 
         /// RunFileName    - the name of the submission script
         /// Note that not all these are serialized
         enum Field { 
                 InputFileName,           // 0
                 OutputFileName, 
                 AuxFileName, 
                 EspFileName, 
                 MoFileName, 
                 DensityFileName,         // 5
                 ErrorFileName, 
                 BatchFileName, 
                 RunFileName, 
                 LocalWorkingDirectory, 
                 RemoteWorkingDirectory,  // 10
                 InputString, 
                 Charge, 
                 Multiplicity, 
                 Coordinates, 
                 CoordinatesFsm,          // 15
                 Constraints, 
                 ScanCoordinates,
                 EfpFragments, 
                 EfpParameters, 
                 ExternalCharges          // 20
              };
            

         QChemJobInfo() : m_charge(0), m_multiplicity(1), m_localFilesExist(false),
           m_promptOnOverwrite(true), m_efpOnlyJob(false), m_moleculePointer(0) { }

         QChemJobInfo(QChemJobInfo const& that) : JobInfo(that) { copy(that); }

         /// Serialization functions that are used to reconstruct the contents of the 
         /// ProcessMonitor on restarting IQmol.  Note that it is assumed that the
         /// Process has been submitted which means we have finished with the input
         /// generator.  This obviates the need to serialize the Coordinates and 
         /// Constraints fields which are currently only requred by the input generator.
         QVariantList toQVariantList() const;
         bool fromQVariantList(QVariantList const&);

         void set(Field const field, QString const& value);
         void set(Field const field, int const& value);

         QString get(Field const field) const;

         QString getRemoteFilePath(Field const) const;
         QString getLocalFilePath(Field const) const;

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

         QChemJobInfo& operator=(QChemJobInfo const& that) {
            if (this != &that) copy(that); return *this;
         }

         void  setMoleculePointer(void* moleculePointer) { m_moleculePointer = moleculePointer; }
         void* moleculePointer() const { return m_moleculePointer; }

      private:
         void copy(QChemJobInfo const&);
         /// Generic object to hold the data
         QMap<Field,QString> m_data;
         int  m_charge;
         int  m_multiplicity;
         bool m_localFilesExist;
         bool m_promptOnOverwrite;
         bool m_efpOnlyJob;

         void* m_moleculePointer;

   };

} } // end namespace IQmol::Process


#endif
