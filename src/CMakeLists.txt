set(IQmol_Main_HEADERS
    Main/AboutDialog.h
    Main/FragmentTable.h
    Main/HelpBrowser.h
    Main/IQmolApplication.h
    Main/MainWindow.h
    Main/PeriodicTable.h
    Main/PreferencesBrowser.h
    Main/ToolBar.h
    Main/Viewer.h
    Main/ViewerModel.h
    Main/ViewerModelView.h
)

set(IQmol_Main_SOURCES
    Main/FragmentTable.C
    Main/HelpBrowser.C
    Main/IQmolApplication.C
    Main/main.C
    Main/MainWindow.C
    Main/PeriodicTable.C
    Main/PreferencesBrowser.C
    Main/ToolBar.C
    Main/Viewer.C
    Main/ViewerModel.C
    Main/ViewerModelView.C
    Main/symmol.f90
)

set(IQmol_Main_FORMS
    Main/AboutDialog.ui
    Main/FragmentTable.ui
    Main/HelpBrowser.ui
    Main/PeriodicTable.ui
    Main/PeriodicTableMac.ui
    Main/PreferencesBrowser.ui
    Main/ToolBar.ui
)
if(APPLE)
    set(IQmol_FORMS ${IQmol_Main_FORMS} Main/PeriodicTableMac.ui)
else(APPLE)
    set(IQmol_FORMS ${IQmol_Main_FORMS} Main/PeriodicTable.ui)
endif(APPLE)

set(IQmol_RESOURCES
    Main/IQmol.qrc
)

set(IQmol_Old_HEADERS
    Old/Animator.h
    Old/BasicServer.h
    Old/BoundingBoxDialog.h
    Old/GridEvaluator.h
    Old/GridInfoDialog.h
    Old/HostDelegate.h
    Old/JobInfo.h
    Old/LocalConnectionThread.h
    Old/LocalHost.h
    Old/LogMessageDialog.h
    Old/MarchingCubes.h
    Old/MeshDecimator.h
    Old/ParseJobFiles.h
    Old/PBSServer.h
    #Old/Process.h
    Old/ProcessMonitor.h
    Old/ProgressDialog.h
    Old/QCProcess.h
    Old/RemoteHost.h
    Old/SecureConnection.h
    Old/SecureConnectionThread.h
    Old/Server.h
    Old/ServerDelegate.h
    Old/ServerDialog.h
    Old/ServerListDialog.h
    Old/ServerOptionsDialog.h
    Old/ServerQueueDialog.h
    Old/ServerTask.h
    Old/SGEServer.h
    Old/ShaderDialog.h
    Old/Snapshot.h
    Old/Spline.h
    Old/SurfaceAnimatorDialog.h
    Old/SymmetryToleranceDialog.h
    Old/Threaded.h
)

set(IQmol_Old_SOURCES
    Old/Animator.C
    Old/AtomicDensity.C
    Old/BasicServer.C
    Old/BoundingBoxDialog.C
    Old/BuildAtomHandler.C
    Old/BuildEfpFragmentHandler.C
    Old/BuildFunctionalGroupHandler.C
    Old/BuildHandler.C
    Old/BuildMoleculeFragmentHandler.C
    Old/ColorGrid.C
    Old/Cursors.C
    Old/gl2ps.C
    Old/GLShape.C
    Old/GLShapeLibrary.C
    Old/GLSLmath.C
    Old/GridEvaluator.C
    Old/GridInfoDialog.C
    Old/IQmol.C
    Old/JobInfo.C
    Old/Lebedev.C
    Old/LocalConnectionThread.C
    Old/LocalHost.C
    Old/LogMessageDialog.C
    Old/ManipulatedFrameSetConstraint.C
    Old/ManipulateHandler.C
    Old/ManipulateSelectionHandler.C
    Old/MarchingCubes.C
    Old/MeshDecimator.C
    Old/ParseJobFiles.C
    Old/PBSServer.C
    #Old/Process.C
    Old/ProcessMonitor.C
    Old/ProgressDialog.C
    Old/QCProcess.C
    Old/ReindexAtomsHandler.C
    Old/RemoteHost.C
    Old/SecureConnection.C
    Old/SecureConnectionThread.C
    Old/SelectHandler.C
    Old/Server.C
    Old/ServerDialog.C
    Old/ServerListDialog.C
    Old/ServerOptionsDialog.C
    Old/ServerQueue.C
    Old/ServerQueueDialog.C
    Old/ServerRegistry.C
    Old/ServerTask.C
    Old/SGEServer.C
    Old/ShaderDialog.C
    Old/ShaderLibrary.C
    Old/Snapshot.C
    Old/SpatialProperty.C
    Old/Spline.C
    Old/SurfaceAnimatorDialog.C
    Old/SymmetryToleranceDialog.C
    #Old/System.C
    Old/UndoCommands.C
)

set(IQmol_Old_FORMS
    Old/BoundingBoxDialog.ui
    Old/GetVaultKeyDialog.ui
    Old/GridInfoDialog.ui
    Old/LogMessageDialog.ui
    Old/ProcessMonitor.ui
    Old/ProgressDialog.ui
    Old/ServerDialog.ui
    Old/ServerListDialog.ui
    Old/ServerOptionsDialog.ui
    Old/ServerQueueDialog.ui
    Old/ShaderDialog.ui
    Old/Snapshot.ui
    Old/SurfaceAnimatorDialog.ui
    Old/SymmetryToleranceDialog.ui
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
    Configurator/MoleculeConfigurator.h
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
    Configurator/MoleculeConfigurator.C
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
    Configurator/MoleculeConfigurator.ui
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
    Data/MolecularOrbitalsList.C
    Data/MultipoleExpansion.C
    Data/NmrReference.C
    Data/PointCharge.C
    Data/RemSectionData.C
    Data/Shell.C
    Data/Surface.C
    Data/SurfaceInfo.C
    Data/SurfaceType.C
    Data/VibrationalMode.C
    Data/YamlNode.C
)

set(IQmol_Old_GL2PS_SOURCES
    Old/gl2ps.C
)
set_property(SOURCE ${IQmol_Old_GL2PS_SOURCES} APPEND PROPERTY
    COMPILE_DEFINITIONS GL2PS)
set(IQmol_Old_SOURCES ${IQmol_Old_SOURCES} ${IQmol_Old_GL2PS_SOURCES})

set(IQmol_Layer_HEADERS
    Layer/AtomLayer.h
    Layer/AxesLayer.h
    Layer/AxesMeshLayer.h
    Layer/BackgroundLayer.h
    Layer/BondLayer.h
    Layer/ChargeLayer.h
    Layer/ConstraintLayer.h
    Layer/ContainerLayer.h
    Layer/CubeDataLayer.h
    Layer/DipoleLayer.h
    Layer/EfpFragmentLayer.h
    Layer/EfpFragmentListLayer.h
    Layer/FileLayer.h
    Layer/FrequenciesLayer.h
    Layer/GeometryLayer.h
    Layer/GeometryListLayer.h
    Layer/GlobalLayer.h
    Layer/GLObjectLayer.h
    Layer/GroupLayer.h
    Layer/InfoLayer.h
    Layer/Layer.h
    Layer/MolecularOrbitalsLayer.h
    Layer/MoleculeLayer.h
    Layer/PrimitiveLayer.h
    Layer/SurfaceLayer.h
)

set(IQmol_Layer_SOURCES
    Layer/AtomLayer.C
    Layer/AxesLayer.C
    Layer/AxesMeshLayer.C
    Layer/BackgroundLayer.C
    Layer/BondLayer.C
    Layer/ChargeLayer.C
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
    Layer/MoleculeLayer.C
    Layer/PrimitiveLayer.C
    Layer/SurfaceLayer.C
)

set(IQmol_Network_HEADERS
    Network/Connection.h
    Network/HttpConnection.h
    Network/HttpReply.h
    Network/LocalConnection.h
    Network/LocalReply.h
    Network/Reply.h
    Network/SshConnection.h
    Network/SshReply.h
)

set(IQmol_Network_SOURCES
    Network/Connection.C
    Network/HttpConnection.C
    Network/HttpReply.C
    Network/LocalConnection.C
    Network/LocalReply.C
    Network/Network.C
    Network/SshConnection.C
    Network/SshReply.C
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
    Parser/YamlParser.C
    Parser/ZMatrixCoordinatesParser.C
)

set(IQmol_Process_HEADERS
    Process/Job.h
    Process/JobMonitor.h
    Process/QChemJobInfo.h
    Process/QueueOptionsDialog.h
    Process/QueueResources.h
    Process/QueueResourcesDialog.h
    Process/QueueSystem.h
    Process/Server2.h
    Process/ServerConfiguration.h
    Process/ServerConfigurationDialog.h
    Process/ServerConfigurationListDialog.h
    Process/ServerRegistry2.h
    Process/SshFileDialog.h
    Process/SystemDependent.h
)

set(IQmol_Process_SOURCES
    Process/Job.C
    Process/JobMonitor.C
    Process/QChemJobInfo.C
    Process/QueueOptionsDialog.C
    Process/QueueResources.C
    Process/QueueResourcesDialog.C
    Process/Server2.C
    Process/ServerConfiguration.C
    Process/ServerConfigurationDialog.C
    Process/ServerConfigurationListDialog.C
    Process/ServerRegistry2.C
    Process/SshFileDialog.C
    Process/SystemDependent.C
)

set(IQmol_Process_FORMS
    Process/JobMonitor.ui
    Process/QueueOptionsDialog.ui
    Process/QueueResourcesDialog.ui
    Process/ServerConfigurationDialog.ui
    Process/ServerConfigurationListDialog.ui
    Process/SshFileDialog.ui
)

set(IQmol_Util_HEADERS
    Util/ColorGradientDialog.h
#    Util/Matrix.h
    Util/QMsgBox.h
    Util/Task.h
    Util/Timer.h
)

set(IQmol_Util_SOURCES
    Util/Align.C
    Util/ColorGradient.C
    Util/ColorGradientDialog.C
    Util/EulerAngles.C
    Util/Function.C
    Util/Matrix.C
    Util/Preferences.C
    Util/qcprot.C
    Util/QMsgBox.C
    Util/RemoveDirectory.C
    Util/ScanDirectory.C
    Util/SetButtonColor.C
    Util/Task.C
    Util/Timer.C
    Util/WriteToTemporaryFile.C
)

set(IQmol_Util_FORMS
    Util/ColorGradientDialog.ui
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
    Qui/Actions.h
    Qui/Conditions.h
    Qui/ExternalChargesSection.h
    Qui/GeometryConstraint.h
    Qui/InputDialog.h
    Qui/KeywordSection.h
    Qui/LJParametersSection.h
    Qui/Logic.h
    Qui/MoleculeSection.h
    Qui/Option.h
    Qui/OptionDatabase.h
    Qui/OptionDatabaseForm.h
    Qui/OptionEditors.h
    Qui/OptionRegister.h
    Qui/QtNode.h
    Qui/QuiMolecule.h
)

set(IQmol_QUI_HEADERS2
    Old/JobInfo.h
)

set(IQmol_QUI_SOURCES
    Qui/Actions.C
    Qui/ExternalChargesSection.C
    Qui/GeometryConstraint.C
    Qui/InputDialog.C
    Qui/InputDialogLogic.C
    Qui/KeywordSection.C
    Qui/LJParametersSection.C
    Qui/MoleculeSection.C
    Qui/Option.C
    Qui/OptionDatabase.C
    Qui/OptionDatabaseForm.C
    Qui/OptionEditors.C
    Qui/OptSection.C
    Qui/QCJob.C
    Qui/Qui.C
    Qui/QuiMolecule.C
    Qui/ReadInput.C
    Qui/RemSection.C
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

set(IQmol_Yaml_HEADERS
    Yaml/collectionstack.h
    Yaml/directives.h
    Yaml/emitterstate.h
    Yaml/emitterutils.h
    Yaml/exp.h
    Yaml/indentation.h
    Yaml/nodebuilder.h
    Yaml/nodeevents.h
    Yaml/ptr_stack.h
    Yaml/ptr_vector.h
    Yaml/regex.h
    Yaml/regeximpl.h
    Yaml/scanner.h
    Yaml/scanscalar.h
    Yaml/scantag.h
    Yaml/setting.h
    Yaml/singledocparser.h
    Yaml/stream.h
    Yaml/streamcharsource.h
    Yaml/stringsource.h
    Yaml/tag.h
    Yaml/token.h
)

set(IQmol_Yaml_SOURCES
    Yaml/binary.cpp
    Yaml/convert.cpp
    Yaml/directives.cpp
    Yaml/emit.cpp
    Yaml/emitfromevents.cpp
    Yaml/emitter.cpp
    Yaml/emitterstate.cpp
    Yaml/emitterutils.cpp
    Yaml/exp.cpp
    Yaml/memory.cpp
    Yaml/node.cpp
    Yaml/node_data.cpp
    Yaml/nodebuilder.cpp
    Yaml/nodeevents.cpp
    Yaml/null.cpp
    Yaml/ostream_wrapper.cpp
    Yaml/parse.cpp
    Yaml/regex.cpp
    Yaml/scanner.cpp
    Yaml/scanscalar.cpp
    Yaml/scantag.cpp
    Yaml/scantoken.cpp
    Yaml/simplekey.cpp
    Yaml/singledocparser.cpp
    Yaml/stream.cpp
    Yaml/tag.cpp
    Yaml/yaml_parser.cpp
)

qt4_wrap_cpp(IQmol_HEADERS_MOC
    ${IQmol_HEADERS}
    ${IQmol_Main_HEADERS}
    ${IQmol_Configurator_HEADERS}
    ${IQmol_Layer_HEADERS}
    ${IQmol_Network_HEADERS}
    ${IQmol_Old_HEADERS}
    ${IQmol_Parser_HEADERS}
    ${IQmol_Process_HEADERS}
    ${IQmol_Util_HEADERS})

qt4_wrap_cpp(IQmol_QUI_HEADERS_MOC
    ${IQmol_QUI_HEADERS})

qt4_wrap_cpp(IQmol_QUI_HEADERS2_MOC
    ${IQmol_QUI_HEADERS2})

qt4_wrap_ui(IQmol_FORMS_HEADERS
    ${IQmol_FORMS}
    ${IQmol_Main_FORMS}
    ${IQmol_Configurator_FORMS}
    ${IQmol_Old_FORMS}
    ${IQmol_Process_FORMS}
    ${IQmol_Util_FORMS})

qt4_wrap_ui(IQmol_QUI_FORMS_HEADERS
    ${IQmol_QUI_FORMS})

qt4_add_resources(IQmol_RESOURCES_RCC ${IQmol_RESOURCES})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(Configurator Data Layer Main Network Old Parser Process Util Yaml Yaml/yaml-cpp)

add_library(Configurator ${IQmol_Configurator_SOURCES})
add_library(Data ${IQmol_Data_SOURCES})
add_library(Layer ${IQmol_Layer_SOURCES})
add_library(Network ${IQmol_Network_SOURCES})
add_library(Old ${IQmol_Old_SOURCES})
add_library(Parser ${IQmol_Parser_SOURCES})
add_library(Process ${IQmol_Process_SOURCES})
add_library(Util ${IQmol_Util_SOURCES})
add_library(QUI ${IQmol_QUI_SOURCES})
add_library(Yaml ${IQmol_Yaml_SOURCES})

add_executable(IQmol
    ${IQmol_Main_SOURCES}
    ${IQmol_HEADERS_MOC} ${IQmol_QUI_HEADERS_MOC}
    ${IQmol_FORMS_HEADERS} ${IQmol_QUI_FORMS_HEADERS}
    ${IQmol_RESOURCES_RCC}
)
target_link_libraries(IQmol
    Configurator
    Data
    Layer
    Network
    Old
    Parser
    Process
    Util
    QUI
    Yaml
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

