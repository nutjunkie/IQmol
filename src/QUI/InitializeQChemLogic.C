/*!
 *  \file InitialiseQChemLogic.C
 *  
 *  \brief This file contains all the option logic that does not relate to the
 *  QUI specifically.  This file should be able to be incorporated directly into
 *  the QChem source.
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "OptionRegister.h"
#include "Actions.h"
#include "Conditions.h"



namespace Qui {

typedef QString S;



void InitializeQChemLogic() {

  OptionRegister& reg = OptionRegister::instance();

  NodeT *node, *node2;
  Rule rule;

  node = &reg.get("BASIS");
  node2 = &reg.get("BASIS2");
  node2->addRule(
     If (*node2 == S("6-311+G**"), node->shouldBe("6-311++G(3df,3pd)")));
  node2->addRule(
     If (*node2 == S("rcc-pVTZ"), node->shouldBe("cc-pVTZ")));
  node2->addRule(
     If (*node2 == S("rcc-pVQZ"), node->shouldBe("cc-pVQZ")));
  node2->addRule(
     If (*node2 == S("6-311+G**") || 
         *node2 == S("rcc-pVTZ") || 
         *node2 == S("rcc-pVQZ"), 
          reg.get("DUAL_BASIS_ENERGY").shouldBe("true"),
          reg.get("DUAL_BASIS_ENERGY").shouldBe("false")));


  node = &reg.get("CORRELATION");
   node->addRule(
      If(isPostHF, 
         reg.get("EXCHANGE").shouldBe("HF"))
   );


}


} // end namespace Qui
