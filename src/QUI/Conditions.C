/*!  
 *  \file Conditions.C
 *   
 *  \brief Definitions of the Conditions used in the QChem Logic.
 *   
 *  \author Andrew Gilbert
 *  \date August 2008
 */


#include "Conditions.h"
#include "OptionRegister.h"
#include "QuiString.h"


namespace Qui {

bool isCompoundFunctional() {
   OptionRegister &reg = OptionRegister::instance();
   NodeT& exchange = reg.get("EXCHANGE");
   String value(ToUpper(exchange.getValue()));
   return (value == "B3LYP")   || (value == "B3LYP5") || (value == "EDF1") ||
          (value == "B3PW91")  || (value == "EDF2")   || (value == "HCTH") ||
          (value == "BECKE97") || (value == "BMK")    || (value == "M05")  ||
          (value == "M06");
}


bool isPostHF() {
   OptionRegister &reg = OptionRegister::instance();
   NodeT& correlation = reg.get("CORRELATION");
   String value(ToUpper(correlation.getValue()));
   return (value == "MP2")     || (value == "MP3")       || (value == "MP4") ||
          (value == "MP4SDQ")  || (value == "LOCAL_MP2") || (value == "RIMP2") ||
          (value == "SOSMP2")  || (value == "MOSMP2")    || (value == "RILMP2") ||
          (value == "CCD")     || (value == "CCD(2)")    || (value == "CCSD") ||
          (value == "CCSD(T)") || (value == "CCSD(2)")   || (value == "QCCD") ||
          (value == "VQCCD")   || (value == "QCISD")     || (value == "QCISD(T)") ||
          (value == "OD")      || (value == "OD(T)")     || (value == "OD(2)") ||
          (value == "VOD")     || (value == "VOD(2)");
}


bool requiresFirstDerivatives() {
   OptionRegister &reg = OptionRegister::instance();
   NodeT& job_type = reg.get("JOB_TYPE");
   String value(ToUpper(job_type.getValue()));
   return (value == "GEOMETRY") || 
          (value == "TRANSITION STATE") || 
          (value == "REACTION PATH") || 
          (value == "AB INITIO MD") || 
          (value == "FORCES");
}


bool requiresSecondDerivatives() {
   OptionRegister &reg = OptionRegister::instance();
   NodeT& job_type = reg.get("JOB_TYPE");
   String value(ToUpper(job_type.getValue()));
   return (value == "FREQUENCIES");
}


bool requiresDerivatives() {
   return requiresFirstDerivatives() || requiresSecondDerivatives();
}



} // end namespace Qui
