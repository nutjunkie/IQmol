/*!
 *  \file InputDialogLogic.C
 *  
 *  \brief This is a member function of the QChem::InputDialog class which
 *  controls the setup of the logic associated directly with the interface.
 *  Logic associated with the QChem executable should go into the routine
 *  InitizeQChemLogic
 *  
 *  \author Andrew Gilbert
 *  \date   August 2008
 */

#include "OptionRegister.h"
#include "Actions.h"
#include "Conditions.h"
#include "InputDialog.h"


// Note: when testing for activated checkboxes, true is 2, false is 0
// Note: Comparrison strings are case-sensitive.  Check the database for consistency
// Note: The node logic is performed using pointers as we don't want to invoke
//        copy constructors on the nodes as the logic will get messed up.


namespace Qui {

typedef QString S;


void InputDialog::initializeQuiLogic() 
{
   // These are just for convenience
   QString QtTrue(QString::number(Qt::Checked));
   QString QtFalse(QString::number(Qt::Unchecked));

   OptionRegister& reg = OptionRegister::instance();

   Rule rule;


   QtNode* basis(&reg.get("BASIS"));
   QtNode* basis2(&reg.get("BASIS2"));
   QtNode* ecp(&reg.get("ECP"));

   rule = If(*basis2 == S("6-311+G**"), basis->shouldBe("6-311++G(3df,3pd)"));
   basis2->addRule(rule);
      
   rule = If(*basis2 == S("rcc-pVTZ"), basis->shouldBe("cc-pVTZ"));
   basis2->addRule(rule);
      
   rule = If(*basis2 == S("rcc-pVQZ"), basis->shouldBe("cc-pVQZ"));
   basis2->addRule(rule);

   rule = If(*basis2 != S("None"), ecp->shouldBe("None") );
   basis2->addRule(rule);
 
   rule = If(*ecp != S("None"), basis->makeSameAs(ecp));
   ecp->addRule(rule);

   rule = If(*basis2 == S("6-311+G**") ||  
             *basis2 == S("rcc-pVTZ")  ||  
             *basis2 == S("rcc-pVQZ"), 
             reg.get("DUAL_BASIS_ENERGY").shouldBe("true"),
             reg.get("DUAL_BASIS_ENERGY").shouldBe("false"));
   basis2->addRule(rule);



   QtNode* jobType(&reg.get("JOB_TYPE"));

   rule = If(requiresDerivatives, 
             reg.get("THRESH").shouldBe("12"), 
             reg.get("THRESH").shouldBe("8"));
   jobType->addRule(rule);
      
   rule = If(requiresDerivatives, 
             reg.get("SCF_CONVERGENCE").shouldBe("8"),
             reg.get("SCF_CONVERGENCE").shouldBe("5"));
   jobType->addRule(rule);

 

   QtNode* exchange(&reg.get("EXCHANGE"));
   QtNode* correlation(&reg.get("CORRELATION"));
   QtNode* cis_n_roots(&reg.get("CIS_N_ROOTS"));

   rule = If(*exchange == S("User-defined"), Disable(m_ui.correlation));
   exchange->addRule(rule);

   rule = If(isCompoundFunctional, Disable(m_ui.correlation), Enable(m_ui.correlation) );
   exchange->addRule(rule);
        
   rule = If(*correlation == S("MP2"), Enable(m_ui.cd_algorithm), Disable(m_ui.cd_algorithm));
   correlation->addRule(rule);

   rule = If(isPostHF, exchange->shouldBe("HF"));
   correlation->addRule(rule);

   rule = If(*correlation == S("RIMP2")      || 
             *correlation == S("SOSMP2")     || 
             *correlation == S("MOSMP2")     ||
             *correlation == S("RICIS(D)")   ||
             *correlation == S("SOSCIS(D)")  ||
             *correlation == S("SOSCIS(D0)") ||

             *correlation == S("CCD")        ||
             *correlation == S("CCD(2)")     ||
             *correlation == S("CCSD")       ||
             *correlation == S("CCSD(T)")    ||
             *correlation == S("CCSD(2)")    ||
             *correlation == S("CCSD(dT)")   ||
             *correlation == S("CCSD(fT)")   ||
             *correlation == S("QCCD")       ||
             *correlation == S("QCISD")      ||
             *correlation == S("QCISD(T)")   ||
             *correlation == S("OD")         ||
             *correlation == S("OD(T)")      ||
             *correlation == S("OD(2)")      ||
             *correlation == S("VOD")        ||
             *correlation == S("VOD(2)")     ||
             *correlation == S("VQCCD")      ||

             *exchange    == S("XYGJOS")     ||
             *exchange    == S("LXYGJOS"),
             Enable(m_ui.auxiliary_basis), 
             Disable(m_ui.auxiliary_basis) 
          );

   correlation->addRule(rule);
   exchange->addRule(rule);

   rule = If(*correlation == S("CIS(D)")    || 
             *correlation == S("RICIS(D)")  || 
             *correlation == S("SOSCIS(D)") || 
             *correlation == S("SOSCIS(D0)"), cis_n_roots->shouldBeAtLeast("1"));
             
   correlation->addRule(rule);



 
   QtNode* node;
   QtNode* node2;

   // Setup -> Frequencies
   node = &reg.get("ANHARMONIC");
   node->addRule(
      If(*node == QtTrue,
         Enable(m_ui.vci), 
         Disable(m_ui.vci) 
      )
   );


   // Setup -> Ab Initio MD
   node = &reg.get("AIMD_METHOD");
   node->addRule(
      If(*node == S("BOMD"), 
         Disable(m_ui.deuterate), 
         Enable(m_ui.deuterate) 
      )
   );
   node = &reg.get("AIMD_INITIAL_VELOCITIES");
   node->addRule(
      If(*node == QString("Thermal"), 
         Enable(m_ui.aimd_temperature), 
         Disable(m_ui.aimd_temperature) 
      )
   );


   // Setup -> Transition State
   node = &reg.get("JOB_TYPE");
   node->addRule(
      If(*node == S("Transition State"),  
         Enable(m_ui.geom_opt_mode), 
         Disable(m_ui.geom_opt_mode) 
      )
   );




   // Advanced -> SCF Control
   node = &reg.get("SCF_ALGORITHM");
   node->addRule(
      If(*node == S("DM"),  
         Enable(m_ui.pseudo_canonical), 
         Disable(m_ui.pseudo_canonical) 
      )
   );
   node->addRule(
      If(*node == S("DIIS_DM") || *node == S("DIIS_GDM"),  
         Enable(m_ui.diis_max_cycles)
         + Enable(m_ui.diis_switch_thresh), 
         Disable(m_ui.diis_max_cycles) 
         + Disable(m_ui.diis_switch_thresh) 
      )
   );
   node->addRule(
      If(*node == S("RCA") || *node == S("RCA_DIIS"),  
         Enable(m_ui.rca_print) 
         + Enable(m_ui.rca_max_cycles),
         Disable(m_ui.rca_print) 
         + Disable(m_ui.rca_max_cycles) 
      )
   );
   node->addRule(
      If(*node == S("RCA_DIIS"),  
         Enable(m_ui.rca_switch_thresh), 
         Disable(m_ui.rca_switch_thresh)
      )
   );


   // Advanced -> SCF Control -> DFT
   node = &reg.get("XC_GRID");
   node->addRule(
      If(*node == S("SG-0") || *node == S("SG-1"),  
         Disable(m_ui.qui_radial_grid),
         Enable(m_ui.qui_radial_grid)
      )
   );

   node = &reg.get("DFT_D");
   node->addRule(
      If(*node == S("Chai Head-Gordon"),
         Enable(m_ui.dft_d_a),
         Disable(m_ui.dft_d_a)
      )
   );



   node = &reg.get("INCDFT");
   node->addRule(
      If(*node == QtFalse,  
         Disable(m_ui.incdft_dendiff_thresh)
         + Disable(m_ui.incdft_dendiff_varthresh)
         + Disable(m_ui.incdft_griddiff_thresh)
         + Disable(m_ui.incdft_griddiff_varthresh), 

         Enable(m_ui.incdft_dendiff_thresh)
         + Enable(m_ui.incdft_dendiff_varthresh)
         + Enable(m_ui.incdft_griddiff_thresh)
         + Enable(m_ui.incdft_griddiff_varthresh)
      )
   );


   // Advanced -> SCF Control -> DFT -> Constrained DFT
   node = &reg.get("CDFT");
   node->addRule(
      If(*node == QtTrue, 
         Enable(m_ui.cdft_prediis)
         + Enable(m_ui.cdft_postdiis)
         + Enable(m_ui.cdft_thresh), 

         Disable(m_ui.cdft_prediis) 
         + Disable(m_ui.cdft_postdiis) 
         + Disable(m_ui.cdft_thresh) 
      )
   );


   // Advanced -> SCF Control -> Print Options
   node = &reg.get("QUI_PRINT_ORBITALS");
   node2 = &reg.get("PRINT_ORBITALS");
   node->addRule(
      If(*node == QtTrue,
         Enable(m_ui.print_orbitals) + node2->shouldBe("5"),
         Disable(m_ui.print_orbitals)
      )
   );


   // Advanced -> SCF Control -> PAO
   node = &reg.get("PAO_METHOD");
   node->addRule(
      If(*node == S("PAO"), 
         Disable(m_ui.epao_iterate)
         + Disable(m_ui.epao_weights),
         Enable(m_ui.epao_iterate) 
         + Enable(m_ui.epao_weights) 
      )
   );


   // Advanced -> Wavefunction Analysis
   QtNode* dma(&reg.get("DMA"));
   rule = If(*node == QtTrue, Enable(m_ui.dma_midpoints), Disable(m_ui.dma_midpoints));
   dma->addRule(rule);

  
   // Advanced -> Wavefunction Analysis -> Plots
   node = &reg.get("PLOTS_GRID");
   node->addRule(
      If(*node == S("None"),  
         Disable(m_ui.plots_property)
         + Disable(m_ui.qui_plots_points), 
         Enable(m_ui.plots_property)
         + Enable(m_ui.qui_plots_points) 
      )
   );
   node->addRule(
      If(*node == S("User-defined"),  
         boost::bind(&InputDialog::printSection, this, "plots", true),
         boost::bind(&InputDialog::printSection, this, "plots", false)
      )
   );
   node->addRule(
      If(*node == S("Read from file"),  
         Enable(m_ui.qui_plots_points), 
         Disable(m_ui.qui_plots_points) 
      )
   );


   // Advanced -> Wavefunction Analysis -> Intracules
   node = &reg.get("INTRACULE");
   node->addRule(
      If(*node == QtTrue,
         Enable(m_ui.intracule_method)
         + Enable(m_ui.intracule_grid)
         + Enable(m_ui.intracule_j_series_limit)
         + Enable(m_ui.intracule_i_series_limit)
         + Enable(m_ui.intracule_wigner_series_limit)
         + Enable(m_ui.intracule_conserve_memory),

         Disable(m_ui.intracule_method)
         + Disable(m_ui.intracule_grid)
         + Disable(m_ui.intracule_j_series_limit)
         + Disable(m_ui.intracule_i_series_limit)
         + Disable(m_ui.intracule_wigner_series_limit)
         + Disable(m_ui.intracule_conserve_memory) 
      )
   );


   // Advanced -> Large Molecule Methods -> CFMM
   node = &reg.get("GRAIN");
   node->addRule(
      If(*node == S("Off"), 
         Disable(m_ui.cfmm_order)
         + Disable(m_ui.lin_k),  
         Enable(m_ui.cfmm_order) 
         + Enable(m_ui.lin_k) 
      )
   );
   // These are around the other way as the default is cfmm on
   node = &reg.get("QUI_CFMM");
   node->addRule(
      If(*node == QtFalse, 
         Disable(m_ui.cfmm_grain)
         + Disable(m_ui.cfmm_order)
         + Disable(m_ui.lin_k), 
         Enable(m_ui.cfmm_grain) 
         + Enable(m_ui.cfmm_order) 
         + Enable(m_ui.lin_k) 
      )
   );
/*
   node = &reg.get("JOB_TYPE");
   node->addRule(
      If(requiresDerivatives, 
         SetValue(m_ui.cfmm_order,25),
         SetValue(m_ui.cfmm_order,15)
      )
   );
*/


   // Advanced -> Large Molecule Methods -> FTC
   node = &reg.get("FTC");
   node->addRule(
      If(*node == QtTrue, 
         Enable(m_ui.ftc_fast)
         + Enable(m_ui.ftc_class_thresh_order)
         + Enable(m_ui.ftc_class_thresh_mult), 
         Disable(m_ui.ftc_fast) 
         + Disable(m_ui.ftc_class_thresh_order) 
         + Disable(m_ui.ftc_class_thresh_mult) 
      )
   );


   // Advanced -> Large Molecule Methods -> CASE
   node = &reg.get("INTEGRAL_2E_OPR");
   node->addRule(
     If(*node == QtTrue, 
        Enable(m_ui.omega), 
        Disable(m_ui.omega) 
     )
   );


   // Advanced -> QMMM
   node = &reg.get("QMMM");
   node->addRule(
     If(*node == QtTrue, 
        Enable(m_ui.qmmm_charges)
        + Enable(m_ui.qmmm_print)
        + Enable(m_ui.link_atom_projection),
        Disable(m_ui.qmmm_charges)
        + Disable(m_ui.qmmm_print)
        + Disable(m_ui.link_atom_projection)
     )
   );

   node2 = &reg.get("JOB_TYPE");
   rule = If(*node == QtTrue && *node2 == S("Frequencies"),
             Enable(m_ui.qmmm_full_hessian),
             Disable(m_ui.qmmm_full_hessian)
          );
   
   node->addRule(rule);
   node2->addRule(rule);


   node  = &reg.get("QUI_SECTION_EXTERNAL_CHARGES");
   node->addRule(
      If(*node == QtTrue, 
        reg.get("SYMMETRY_INTEGRAL").shouldBe(QtFalse)
        + reg.get("SYMMETRY_IGNORE").shouldBe(QtTrue)
      )
   );

   node2 = &reg.get("JOB_TYPE");
   rule = If(*node == QtTrue && *node2 == S("Geometry"),
          reg.get("GEOM_OPT_IPROJ").shouldBe(QtFalse)
          );
   node->addRule(rule);
   node2->addRule(rule);


   // Advanced -> Correlated Methods
   node = &reg.get("QUI_FROZEN_CORE");
   node->addRule(
      If(*node == QtTrue,
         Disable(m_ui.n_frozen_core)
         + Disable(m_ui.n_frozen_virtual), 
         Enable(m_ui.n_frozen_core)
         + Enable(m_ui.n_frozen_virtual)
      ) 
   );


   // Advanced -> Correlated Methods -> Coupled-Cluster
   /*node = &reg.get("CC_PROPERTIES");
   node->addRule(
      If(*node == QtTrue,
         Enable(m_ui.cc_two_particle_properties)
         + Enable(m_ui.cc_amplitude_response)
         + Enable(m_ui.cc_full_response), 
         Disable(m_ui.cc_two_particle_properties)
         + Disable(m_ui.cc_amplitude_response)
         + Disable(m_ui.cc_full_response)
      ) 
   );

   node = &reg.get("CC_MP2NO_GUESS");
   node->addRule(
      If(*node == QtTrue,
         Enable(m_ui.cc_mp2no_grad) + Enable(m_ui.threads), 
         Disable(m_ui.cc_mp2no_grad) + Disable(m_ui.threads)
      ) 
   );*/


   // Advanced -> Correlated Methods -> Coupled-Cluster -> Convergence
   /*node = &reg.get("CC_DIIS");
   node->addRule(
      If(*node == S("Switch"),
         Enable(m_ui.cc_diis12_switch), 
         Disable(m_ui.cc_diis12_switch)
      ) 
   );*/


   // Advanced -> Correlated Methods -> Active Space
   node = &reg.get("CC_RESTART");
   node->addRule(
      If(*node == QtTrue,
         Enable(m_ui.cc_restart_no_scf), 
         Disable(m_ui.cc_restart_no_scf)
      ) 
   );


   // Advanced -> Excited States -> CIS
   node = &reg.get("JOB_TYPE");
   node->addRule(
      If(*node == S("Geometry") || *node == S("Frequencies") 
                                || *node == S("Forces"), 
         Enable(m_ui.cis_state_derivative), 
         Disable(m_ui.cis_state_derivative) 
      )
   );
   node = &reg.get("CIS_GUESS_DISK");
   node->addRule(
      If(*node == QtTrue,
         Enable(m_ui.cis_guess_disk_type), 
         Disable(m_ui.cis_guess_disk_type) 
      )
   );


   // Advanced -> Excited States -> TD DFT
   node = &reg.get("CIS_RAS");
   node->addRule(
      If(*node == QtTrue, 
         Enable(m_ui.cis_ras_type)
         + Enable(m_ui.cis_ras_cutoff_occupied)
         + Enable(m_ui.cis_ras_cutoff_virtual)
         + Enable(m_ui.cis_ras_print),
         Disable(m_ui.cis_ras_type) 
         + Disable(m_ui.cis_ras_cutoff_occupied) 
         + Disable(m_ui.cis_ras_cutoff_virtual) 
         + Disable(m_ui.cis_ras_print) 
      )
   );


   // Advanced -> Excited States -> EOM -> Properties
 /*  node = &reg.get("CC_EOM_PROPERTIES");
   node->addRule(
      If(*node == QtTrue, 
         Enable(m_ui.cc_eom_transition_properties)
         + Enable(m_ui.cc_eom_amplitude_response)
         + Enable(m_ui.cc_eom_full_response)
         + Enable(m_ui.cc_eom_two_particle_properties), 
         Disable(m_ui.cc_eom_transition_properties) 
         + Disable(m_ui.cc_eom_amplitude_response) 
         + Disable(m_ui.cc_eom_full_response) 
         + Disable(m_ui.cc_eom_two_particle_properties) 
      )
   );*/


   // Advanced -> Excited States -> XOPT
   /*node = &reg.get("QUI_XOPT1");
   node2 = &reg.get("QUI_XOPT2");
   node->addRule(
      If(*node == QtTrue,
         Enable(m_ui.qui_xopt_spin1)
         + Enable(m_ui.qui_xopt_state1)
         + Enable(m_ui.qui_xopt_irrep1)
         + Enable(m_ui.qui_xopt_spin2)
         + Enable(m_ui.qui_xopt_state2)
         + Enable(m_ui.qui_xopt_irrep2)
         + Enable(m_ui.xopt_seam_only),
         Disable(m_ui.qui_xopt_spin1)
         + Disable(m_ui.qui_xopt_state1)
         + Disable(m_ui.qui_xopt_irrep1)
         + Disable(m_ui.qui_xopt_spin2)
         + Disable(m_ui.qui_xopt_state2)
         + Disable(m_ui.qui_xopt_irrep2)
         + Disable(m_ui.xopt_seam_only)
      )
   );
   node->addRule(
      If(*node == QtTrue,  
        node2->shouldBe(QtTrue), 
        node2->shouldBe(QtFalse)
      )
   );*/


   // Advanced -> Solvation Models -> ChemSol
   node = &reg.get("CHEMSOL");
   node->addRule(
      If(*node == QtTrue, 
         Enable(m_ui.chemsol_nn)
         + Enable(m_ui.chemsol_print)
         + Enable(m_ui.chemsol_efield)
         + Enable(m_ui.chemsol_read_vdw), 
         Disable(m_ui.chemsol_nn) 
         + Disable(m_ui.chemsol_print)
         + Disable(m_ui.chemsol_efield)
         + Disable(m_ui.chemsol_read_vdw) 
      )
   );


   QtNode* dielectricOnsager(&reg.get("QUI_SOLVENT_DIELECTRIC_ONSAGER"));
   QtNode* dielectricCosmo(&reg.get("QUI_SOLVENT_DIELECTRIC_COSMO"));
   QtNode* solventDielectric(&reg.get("SOLVENT_DIELECTRIC"));

   rule = If(True, solventDielectric->makeSameAs(dielectricOnsager));
   dielectricOnsager->addRule(rule);
   rule = If(True, solventDielectric->makeSameAs(dielectricCosmo));
   dielectricCosmo->addRule(rule);

   QtNode* cosmo(&reg.get("QUI_SOLVENT_COSMO"));
   rule = If(*cosmo == QtTrue, reg.get("SOLVENT_METHOD").shouldBe("COSMO"));
   cosmo->addRule(rule);

   QtNode* pcm(&reg.get("QUI_SOLVENT_PCM"));
   rule = If(*pcm == QtTrue, reg.get("SOLVENT_METHOD").shouldBe("PCM"));
   pcm->addRule(rule);

   QtNode* onsager(&reg.get("QUI_SOLVENT_ONSAGER"));
   rule = If(*onsager == QtTrue, reg.get("SOLVENT_METHOD").shouldBe("SCRF"));
   onsager->addRule(rule);


   rule = If(*cosmo == QtTrue || *pcm == QtTrue || *onsager == QtTrue,
            Enable(m_ui.solvent_method), Disable(m_ui.solvent_method));

   cosmo->addRule(rule);
   pcm->addRule(rule);
   onsager->addRule(rule);
   
   rule = If(*cosmo == QtTrue || *onsager == QtTrue,
            Enable(m_ui.solvent_dielectric), Disable(m_ui.solvent_dielectric));

   cosmo->addRule(rule);
   onsager->addRule(rule);



   // Section logic ----------------------------------------------------------
   node = &reg.get("QUI_TITLE");
   node->addRule(
      If(*node == S(""), 
         boost::bind(&InputDialog::printSection, this, "comment", false),
         boost::bind(&InputDialog::printSection, this, "comment", true)
      )
   );

   node = &reg.get("EXCHANGE");
   node->addRule(
      If(*node == S("User-defined"), 
         boost::bind(&InputDialog::printSection, this, "xc_functional", true),
         boost::bind(&InputDialog::printSection, this, "xc_functional", false)
      )
   );

   node = &reg.get("CHEMSOL_READ_VDW");
   node->addRule(
      If(*node == S("User-defined"), 
         boost::bind(&InputDialog::printSection, this, "van_der_waals", true),
         boost::bind(&InputDialog::printSection, this, "van_der_waals", false)
      )
   );

   node = &reg.get("ECP");
   node->addRule( 
      If(*node == S("User-defined"), 
         boost::bind(&InputDialog::printSection, this, "ecp", true),
         boost::bind(&InputDialog::printSection, this, "ecp", false)
      ) 
   );
           
   node = &reg.get("BASIS");
   node->addRule(
      If(*node == S("User-defined") || *node == S("Mixed"),
         boost::bind(&InputDialog::printSection, this, "basis", true),
         boost::bind(&InputDialog::printSection, this, "basis", false)
      )
   );

   node = &reg.get("AIMD_INITIAL_VELOCITIES");
   node2 = &reg.get("JOB_TYPE");
   rule = If(*node2 == S("Ab Initio MD") && *node == S("Read"),
             boost::bind(&InputDialog::printSection, this, "velocity", true),
             boost::bind(&InputDialog::printSection, this, "velocity", false)
          );
   node->addRule(rule);
   node2->addRule(rule);

   node = &reg.get("CIS_RAS_TYPE");
   node->addRule(
      If(*node == S("User-defined"),
         boost::bind(&InputDialog::printSection, this, "solute", true),
         boost::bind(&InputDialog::printSection, this, "solute", false)
      )
   );

   node = &reg.get("ISOTOPES");
   node->addRule(
      If (*node == QtTrue,
         boost::bind(&InputDialog::printSection, this, "isotopes", true),
         boost::bind(&InputDialog::printSection, this, "isotopes", false)
      )
   );
  
   node = &reg.get("JOB_TYPE");
   node->addRule(
      If (*node == S("Geometry") || *node == S("Reaction Path")
       || *node == S("Transition State"),
         boost::bind(&InputDialog::printSection, this, "opt", true),
         boost::bind(&InputDialog::printSection, this, "opt", false)
      )
   );


   //node = &reg.get("QMMM");
   node = &reg.get("QUI_SECTION_EXTERNAL_CHARGES");
   node->addRule(
      If (*node == QtTrue,
         boost::bind(&InputDialog::updateLJParameters, this)
         + boost::bind(&InputDialog::printSection, this, "lj_parameters", true),
         boost::bind(&InputDialog::printSection, this, "lj_parameters", false)
      )
   );


}

} // end namespace Qui
