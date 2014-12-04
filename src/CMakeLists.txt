set(IQmol_HEADERS
    IQmol/AboutDialog.h
    IQmol/Animator.h
    IQmol/AtomLayer.h
    IQmol/BasicServer.h
    IQmol/BondLayer.h
    IQmol/BoundingBoxDialog.h
    IQmol/ChargeLayer.h
    IQmol/FragmentTable.h
    IQmol/GlobalLayer.h
    IQmol/Gradient.h
    IQmol/GridEvaluator.h
    IQmol/GridInfoDialog.h
    IQmol/HelpBrowser.h
    IQmol/HostDelegate.h
    IQmol/HttpServer.h
    IQmol/HttpThread.h
    IQmol/IQmolApplication.h
    IQmol/JobInfo.h
    IQmol/LocalConnectionThread.h
    IQmol/LocalHost.h
    IQmol/LogMessageDialog.h
    IQmol/MainWindow.h
    IQmol/MarchingCubes.h
    IQmol/MeshDecimator.h
    IQmol/MoleculeConfigurator.h
    IQmol/MoleculeLayer.h
    IQmol/PBSServer.h
    IQmol/ParseJobFiles.h
    IQmol/PasswordVault.h
    IQmol/PeriodicTable.h
    IQmol/PreferencesBrowser.h
    IQmol/PrimitiveLayer.h
    IQmol/Process.h
    IQmol/ProcessMonitor.h
    IQmol/ProgressDialog.h
    IQmol/RemoteHost.h
    IQmol/SGEServer.h
    IQmol/SecureConnection.h
    IQmol/SecureConnectionThread.h
    IQmol/Server.h
    IQmol/ServerDelegate.h
    IQmol/ServerDialog.h
    IQmol/ServerListDialog.h
    IQmol/ServerOptionsDialog.h
    IQmol/ServerQueueDialog.h
    IQmol/ServerTask.h
    IQmol/ShaderDialog.h
    IQmol/Snapshot.h
    IQmol/SshFileDialog.h
    IQmol/SurfaceAnimatorDialog.h
    IQmol/SymmetryToleranceDialog.h
    IQmol/Threaded.h
    IQmol/Timer.h
    IQmol/ToolBar.h
    IQmol/Viewer.h
    IQmol/ViewerModel.h
    IQmol/ViewerModelView.h
    IQmol/WebHost.h
)

set(IQmol_SOURCES
    IQmol/Animator.C
    IQmol/AtomLayer.C
    IQmol/AtomicDensity.C
    IQmol/BasicServer.C
    IQmol/BondLayer.C
    IQmol/BoundingBoxDialog.C
    IQmol/BuildAtomHandler.C
    IQmol/BuildEfpFragmentHandler.C
    IQmol/BuildFunctionalGroupHandler.C
    IQmol/BuildHandler.C
    IQmol/BuildMoleculeFragmentHandler.C
    IQmol/ChargeLayer.C
    IQmol/ColorGrid.C
    IQmol/Cursors.C
    IQmol/EnigmaMachine.C
    IQmol/FragmentTable.C
    IQmol/GLSLmath.C
    IQmol/GLShape.C
    IQmol/GLShapeLibrary.C
    IQmol/Gradient.C
    IQmol/GridEvaluator.C
    IQmol/GridInfoDialog.C
    IQmol/HelpBrowser.C
    IQmol/HttpServer.C
    IQmol/HttpThread.C
    IQmol/IQmol.C
    IQmol/IQmolApplication.C
    IQmol/JobInfo.C
    IQmol/Lebedev.C
    IQmol/LocalConnectionThread.C
    IQmol/LocalHost.C
    IQmol/LogMessageDialog.C
    IQmol/MainWindow.C
    IQmol/ManipulateHandler.C
    IQmol/ManipulateSelectionHandler.C
    IQmol/ManipulatedFrameSetConstraint.C
    IQmol/MarchingCubes.C
    IQmol/MeshDecimator.C
    IQmol/MoleculeConfigurator.C
    IQmol/MoleculeLayer.C
    IQmol/PBSServer.C
    IQmol/ParseJobFiles.C
    IQmol/PasswordVault.C
    IQmol/PeriodicTable.C
    IQmol/PreferencesBrowser.C
    IQmol/PrimitiveLayer.C
    IQmol/Process.C
    IQmol/ProcessMonitor.C
    IQmol/ProgressDialog.C
    IQmol/ReindexAtomsHandler.C
    IQmol/RemoteHost.C
    IQmol/SGEServer.C
    IQmol/SecureConnection.C
    IQmol/SecureConnectionThread.C
    IQmol/SelectHandler.C
    IQmol/Server.C
    IQmol/ServerDialog.C
    IQmol/ServerListDialog.C
    IQmol/ServerOptionsDialog.C
    IQmol/ServerQueue.C
    IQmol/ServerQueueDialog.C
    IQmol/ServerRegistry.C
    IQmol/ServerTask.C
    IQmol/ShaderDialog.C
    IQmol/ShaderLibrary.C
    IQmol/Shell.C
    IQmol/Snapshot.C
    IQmol/SpatialProperty.C
    IQmol/Spline.C
    IQmol/SshFileDialog.C
    IQmol/SurfaceAnimatorDialog.C
    IQmol/SurfaceAnimatorDialog.C
    IQmol/SymmetryToleranceDialog.C
    IQmol/System.C
    IQmol/Timer.C
    IQmol/ToolBar.C
    IQmol/UndoCommands.C
    IQmol/Viewer.C
    IQmol/ViewerModel.C
    IQmol/ViewerModelView.C
    IQmol/WebHost.C
    IQmol/main.C
    IQmol/symmol.f90
)

set(IQmol_FORMS
    IQmol/AboutDialog.ui
    IQmol/BoundingBoxDialog.ui
    IQmol/CustomServerConfigurator.ui
    IQmol/FragmentTable.ui
    IQmol/GetVaultKeyDialog.ui
    IQmol/GradientDialog.ui
    IQmol/GridInfoDialog.ui
    IQmol/HelpBrowser.ui
    IQmol/LogMessageDialog.ui
    IQmol/MoleculeConfigurator.ui
    IQmol/PreferencesBrowser.ui
    IQmol/ProcessMonitor.ui
    IQmol/ProgressDialog.ui
    IQmol/ServerDialog.ui
    IQmol/ServerListDialog.ui
    IQmol/ServerOptionsDialog.ui
    IQmol/ServerQueueDialog.ui
    IQmol/SetPasswordDialog.ui
    IQmol/SetPasswordVaultKeyDialog.ui
    IQmol/ShaderDialog.ui
    IQmol/Snapshot.ui
    IQmol/SshFileDialog.ui
    IQmol/SurfaceAnimatorDialog.ui
    IQmol/SymmetryToleranceDialog.ui
    IQmol/ToolBar.ui
)
if(APPLE)
    set(IQmol_FORMS ${IQmol_FORMS} IQmol/PeriodicTableMac.ui)
else(APPLE)
    set(IQmol_FORMS ${IQmol_FORMS} IQmol/PeriodicTable.ui)
endif(APPLE)

set(IQmol_RESOURCES
    IQmol/IQmol.qrc
)

set(IQmol_Configurator_HEADERS
    Configurator/AxesMeshConfigurator.h
    Configurator/BackgroundConfigurator.h
    Configurator/Configurator.h
    Configurator/ConstraintConfigurator.h
    Configurator/CubeDataConfigurator.h
    Configurator/DipoleConfigurator.h
    Configurator/EfpFragmentListConfigurator.h
    Configurator/FileConfigurator.h
    Configurator/FrequenciesConfigurator.h
    Configurator/GeometryListConfigurator.h
    Configurator/InfoConfigurator.h
    Configurator/MolecularOrbitalsConfigurator.h
    Configurator/SurfaceConfigurator.h
)
   
set(IQmol_Configurator_SOURCES
    Configurator/AxesMeshConfigurator.C
    Configurator/BackgroundConfigurator.C
    Configurator/ConstraintConfigurator.C
    Configurator/CubeDataConfigurator.C
    Configurator/DipoleConfigurator.C
    Configurator/EfpFragmentListConfigurator.C
    Configurator/FileConfigurator.C
    Configurator/FrequenciesConfigurator.C
    Configurator/GeometryListConfigurator.C
    Configurator/InfoConfigurator.C
    Configurator/MolecularOrbitalsConfigurator.C
    Configurator/SurfaceConfigurator.C
)

set(IQmol_Configurator_FORMS
    Configurator/AxesMeshConfigurator.ui
    Configurator/BackgroundConfigurator.ui
    Configurator/CubeDataConfigurator.ui
    Configurator/DipoleConfigurator.ui
    Configurator/EfpFragmentListConfigurator.ui
    Configurator/FileConfigurator.ui
    Configurator/FrequenciesConfigurator.ui
    Configurator/GeometryListConfigurator.ui
    Configurator/InfoConfigurator.ui
    Configurator/MolecularOrbitalsConfigurator.ui
    Configurator/ScalarConstraintConfigurator.ui
    Configurator/SurfaceConfigurator.ui
    Configurator/VectorConstraintConfigurator.ui
)

set(IQmol_Data_SOURCES
    Data/Atom.C
    Data/AtomicProperty.C
    Data/Bank.C
    Data/ChargeMultiplicity.C
    Data/Constraint.C
    Data/Data.C
    Data/DataFactory.C
    Data/EfpFragment.C
    Data/EfpFragmentLibrary.C
    Data/Energy.C
    Data/File.C
    Data/Frequencies.C
    Data/Geometry.C
    Data/GeometryList.C
    Data/GridData.C
    Data/GridSize.C
    Data/Hessian.C
    Data/Mesh.C
    Data/MolecularOrbitals.C
    Data/MultipoleExpansion.C
    Data/PointCharge.C
    Data/RemSectionData.C
    Data/Shell.C
    Data/Surface.C
    Data/SurfaceInfo.C
    Data/SurfaceType.C
    Data/VibrationalMode.C
)

set(IQmol_IQmol_GL2PS_SOURCES
    IQmol/gl2ps.C
)
set_property(SOURCE ${IQmol_IQmol_GL2PS_SOURCES} APPEND PROPERTY
    COMPILE_DEFINITIONS GL2PS)
set(IQmol_SOURCES ${IQmol_SOURCES} ${IQmol_IQmol_GL2PS_SOURCES})

set(IQmol_Layer_HEADERS
    Layer/AxesLayer.h
    Layer/AxesMeshLayer.h
    Layer/BackgroundLayer.h
    Layer/ConstraintLayer.h
    Layer/ContainerLayer.h
    Layer/CubeDataLayer.h
    Layer/DipoleLayer.h
    Layer/EfpFragmentLayer.h
    Layer/EfpFragmentListLayer.h
    Layer/FileLayer.h
    Layer/FrequenciesLayer.h
    Layer/GLObjectLayer.h
    Layer/GeometryLayer.h
    Layer/GeometryListLayer.h
    Layer/GroupLayer.h
    Layer/InfoLayer.h
    Layer/Layer.h
    Layer/MolecularOrbitalsLayer.h
    Layer/SurfaceLayer.h
)

set(IQmol_Layer_SOURCES
    Layer/AxesLayer.C
    Layer/AxesMeshLayer.C
    Layer/BackgroundLayer.C
    Layer/ConstraintLayer.C
    Layer/ContainerLayer.C
    Layer/CubeDataLayer.C
    Layer/DipoleLayer.C
    Layer/EfpFragmentLayer.C
    Layer/EfpFragmentListLayer.C
    Layer/FileLayer.C
    Layer/FrequenciesLayer.C
    Layer/GeometryLayer.C
    Layer/GeometryListLayer.C
    Layer/GroupLayer.C
    Layer/InfoLayer.C
    Layer/Layer.C
    Layer/LayerFactory.C
    Layer/MolecularOrbitalsLayer.C
    Layer/SurfaceLayer.C
)

set(IQmol_Network_HEADERS
    Network/LocalSession.h
    Network/ServerConfigurationDialog.h
    Network/Session.h
    Network/Transaction.h
)

set(IQmol_Network_SOURCES
    Network/ServerConfiguration.C
    Network/ServerConfigurationDialog.C
    Network/ServerRegistry.C
    Network/Session.C
)

set(IQmol_Parser_HEADERS
    Parser/ParseFile.h
)

set(IQmol_Parser_SOURCES
    Parser/CartesianCoordinatesParser.C
    Parser/CubeParser.C
    Parser/EfpFragmentParser.C
    Parser/ExternalChargesParser.C
    Parser/FormattedCheckpointParser.C
    Parser/IQmolParser.C
    Parser/MeshParser.C
    Parser/OpenBabelParser.C
    Parser/ParseFile.C
    Parser/Parser.C
    Parser/QChemInputParser.C
    Parser/QChemOutputParser.C
    Parser/QChemPlotParser.C
    Parser/XyzParser.C
    Parser/ZMatrixCoordinatesParser.C
)

set(IQmol_Util_HEADERS
    Util/QMsgBox.h
    Util/Task.h
)

set(IQmol_Util_SOURCES
    Util/Align.C
    Util/EulerAngles.C
    Util/Function.C
    Util/Matrix.C
    Util/Preferences.C
    Util/QMsgBox.C
    Util/ScanDirectory.C
    Util/SetButtonColor.C
    Util/Task.C
    Util/qcprot.C
)
set(IQmol_Util_QsLog_SOURCES
    Util/QsLogDest.C
    Util/QsLog.C
    Util/QsDebugOutput.C
)
set_property(SOURCE ${IQmol_Util_QsLog_SOURCES} APPEND PROPERTY
    COMPILE_DEFINITIONS QS_LOG_LINE_NUMBERS)
set(IQmol_Util_SOURCES ${IQmol_Util_SOURCES} ${IQmol_Util_QsLog_SOURCES})

set(IQmol_QUI_HEADERS
    QUI/GeometryConstraint.h
    QUI/InputDialog.h
    QUI/OptionDatabaseForm.h
    QUI/OptionEditors.h
    QUI/QtNode.h
    QUI/QuiMolecule.h
)

set(IQmol_QUI_HEADERS2
    IQmol/JobInfo.h
)

set(IQmol_QUI_SOURCES
    QUI/Actions.C
    QUI/ExternalChargesSection.C
    QUI/GeometryConstraint.C
    QUI/InputDialog.C
    QUI/InputDialogLogic.C
    QUI/Job.C
    QUI/KeywordSection.C
    QUI/LJParametersSection.C
    QUI/MoleculeSection.C
    QUI/OptSection.C
    QUI/Option.C
    QUI/OptionDatabase.C
    QUI/OptionDatabaseForm.C
    QUI/OptionEditors.C
    QUI/Qui.C
    QUI/QuiMolecule.C
    QUI/ReadInput.C
    QUI/RemSection.C
)

set(IQmol_QUI_FORMS
    QUI/GeometryConstraintDialog.ui
    QUI/OptionDatabaseForm.ui
    QUI/OptionListEditor.ui
    QUI/OptionNumberEditor.ui
    QUI/QuiMainWindow.ui
)
set_property(SOURCE ${IQmol_QUI_SOURCES} APPEND PROPERTY
    COMPILE_DEFINITIONS QCHEM_UI)

qt4_wrap_cpp(IQmol_HEADERS_MOC ${IQmol_HEADERS} ${IQmol_Configurator_HEADERS}
    ${IQmol_Layer_HEADERS} ${IQmol_Parser_HEADERS} ${IQmol_Util_HEADERS})
qt4_wrap_cpp(IQmol_QUI_HEADERS_MOC ${IQmol_QUI_HEADERS})
qt4_wrap_cpp(IQmol_QUI_HEADERS2_MOC ${IQmol_QUI_HEADERS2})
qt4_wrap_ui(IQmol_FORMS_HEADERS ${IQmol_FORMS} ${IQmol_Configurator_FORMS})
qt4_wrap_ui(IQmol_QUI_FORMS_HEADERS ${IQmol_QUI_FORMS})
qt4_add_resources(IQmol_RESOURCES_RCC ${IQmol_RESOURCES})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(Configurator Data IQmol Layer Network Parser Util)

add_executable(IQmol
    ${IQmol_SOURCES}
    ${IQmol_Configurator_SOURCES}
    ${IQmol_Data_SOURCES}
    ${IQmol_Layer_SOURCES}
#    ${IQmol_Network_SOURCES}
    ${IQmol_Parser_SOURCES}
    ${IQmol_Util_SOURCES}
    ${IQmol_QUI_SOURCES}
    ${IQmol_HEADERS_MOC} ${IQmol_QUI_HEADERS_MOC}
    ${IQmol_FORMS_HEADERS} ${IQmol_QUI_FORMS_HEADERS}
    ${IQmol_RESOURCES_RCC}
)
target_link_libraries(IQmol
    ${OPENBABEL2_LIBRARIES}
    ${OPENMESH_LIBRARIES}
    ${QGLVIEWER_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${LIBSSH2_LIBRARY} ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES} ${ZLIB_LIBRARIES}
)

#add_executable(QUI
#    ${IQmol_QUI_SOURCES} QUI/main.C
#    ${IQmol_QUI_HEADERS_MOC} ${IQmol_QUI_HEADERS2_MOC}
#    ${IQmol_QUI_FORMS_HEADERS}
#    GLSLmath.C Preferences.C ShaderLibrary.C JobInfo.C
#    ${IQmol_Util_SOURCES}
#)
#target_link_libraries(QUI
#    ${OPENBABEL2_LIBRARIES}
#    ${QT_LIBRARIES}
#    ${OPENGL_LIBRARIES}
#)

#set_target_properties(IQmol QUI PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)
set_target_properties(IQmol PROPERTIES RUNTIME_OUTPUT_DIRECTORY bin)
set_target_properties(IQmol PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

#install(TARGETS IQmol QUI DESTINATION bin)
install(TARGETS IQmol DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/fragments
    DESTINATION share/iqmol)
install(FILES QUI/qchem_option.db
    DESTINATION share/iqmol/QUI)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/share/shaders
    DESTINATION share/iqmol)

