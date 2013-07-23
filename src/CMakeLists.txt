set(IQmol_HEADERS
    IQmolApplication.h
    MainWindow.h
    ToolBar.h
    HelpBrowser.h
    PeriodicTable.h
    Preferences.h
    Viewer.h
    AboutDialog.h
    ViewerModel.h
    GLObjectLayer.h
    BaseLayer.h
    MeshLayer.h
    AxesLayer.h
    BackgroundLayer.h
    MoleculeLayer.h
    PrimitiveLayer.h
    AtomLayer.h
    BondLayer.h
    ChargeLayer.h
    SymmetryToleranceDialog.h
    ViewerModelView.h
    EFPFragmentListConfigurator.h 
    Grid.h 
    ProgressDialog.h
    SurfaceLayer.h
    MarchingCubes.h
    CubeDataLayer.h 
    GlobalLayer.h
    MeshConfigurator.h
    InfoLayer.h 
    InfoConfigurator.h
    BaseConfigurator.h
    BackgroundConfigurator.h 
    MoleculeConfigurator.h
    SurfaceConfigurator.h
    DataLayer.h
    FrequenciesConfigurator.h 
    FrequenciesLayer.h
    Animator.h
    ConformerListLayer.h 
    MolecularOrbitalsLayer.h 
    MolecularOrbitalsConfigurator.h
    ConformerListConfigurator.h 
    CubeDataConfigurator.h
    FileLayer.h
    FileConfigurator.h
    DipoleLayer.h 
    DipoleConfigurator.h
    Gradient.h
    ConstraintLayer.h
    ConstraintConfigurator.h
    LogMessageDialog.h 
    Snapshot.h
    PasswordVault.h
    Server.h
    ServerListDialog.h 
    JobInfo.h
    ProcessMonitor.h 
    Process.h
    Timer.h
    SecureConnection.h
    SSHFileConfigurator.h 
    ServerQueueDialog.h  
    Threaded.h
    SecureConnectionThread.h 
    ServerDelegate.h 
    ServerTask.h 
    HostDelegate.h 
    LocalHost.h 
    RemoteHost.h 
    HttpServer.h 
    HttpThread.h 
    WebHost.h 
    BasicServer.h 
    PBSServer.h 
    SGEServer.h 
    FragmentTable.h 
    ServerDialog.h
    ServerOptionsDialog.h 
    ContainerLayer.h 
    GroupLayer.h 
    EFPFragmentLayer.h 
    EFPFragmentListLayer.h 
    Task.h 
    ShaderDialog.h 
    GridInfoDialog.h 
    SurfaceAnimatorDialog.h
    BoundingBoxDialog.h
    LocalConnectionThread.h
)

set(IQmol_SOURCES
   symmol.f90
   IQmolApplication.C  main.C  MainWindow.C  ToolBar.C  Preferences.C  Viewer.C 
   HelpBrowser.C  PeriodicTable.C  ViewerModel.C  MeshLayer.C 
   MeshConfigurator.C  AxesLayer.C  BackgroundLayer.C  MoleculeLayer.C 
   ManipulatedFrameSetConstraint.C  AtomLayer.C  BondLayer.C  ChargeLayer.C 
   UndoCommands.C  SymmetryToleranceDialog.C  ViewerModelView.C  Cursors.C 
   ManipulateHandler.C  SelectHandler.C  ManipulateSelectionHandler.C 
   BuildHandler.C  PrimitiveLayer.C 
   BuildAtomHandler.C  
   BuildFunctionalGroupHandler.C 
   BuildEFPFragmentHandler.C 
   EFPFragmentListConfigurator.C 
   BuildMoleculeFragmentHandler.C 
   Shell.C  Grid.C  
   ProgressDialog.C 
   SurfaceLayer.C  MarchingCubes.C  CubeDataLayer.C  QChemParser.C  InfoLayer.C 
   InfoConfigurator.C  BackgroundConfigurator.C  MoleculeConfigurator.C 
   SurfaceConfigurator.C  BaseParser.C  ExternalChargesParser.C 
   OpenBabelParser.C  FrequenciesConfigurator.C  FrequenciesLayer.C  Animator.C 
   ConformerListLayer.C  FormattedCheckpointParser.C   MolecularOrbitalsLayer.C 
   MolecularOrbitalsConfigurator.C  DipoleLayer.C  ConformerListConfigurator.C 
   CubeDataConfigurator.C  FileConfigurator.C  DipoleConfigurator.C  Gradient.C 
   ColorGrid.C  SpatialProperty.C  AtomicDensity.C  IQmol.C  
   ReindexAtomsHandler.C  ConstraintLayer.C 
   ConstraintConfigurator.C  GLShape.C LogMessageDialog.C  Snapshot.C  
   EnigmaMachine.C  PasswordVault.C  Server.C  ServerListDialog.C 
   ProcessMonitor.C 
   JobInfo.C Process.C  Timer.C  System.C  
   SecureConnection.C  SSHFileConfigurator.C  
   ServerRegistry.C 
   ServerQueue.C  ServerQueueDialog.C  
   SecureConnectionThread.C 
   ServerDialog.C  ServerOptionsDialog.C 
   LocalHost.C 
   RemoteHost.C 
   HttpServer.C 
   WebHost.C 
   HttpThread.C 
   ServerTask.C 
   BasicServer.C 
   PBSServer.C 
   SGEServer.C 
   FragmentTable.C 
#  Sanderson.C  
   GLShapeLibrary.C 
   GroupLayer.C 
   EFPFragmentLayer.C 
   EFPFragmentListLayer.C 
   EFPFragmentParser.C 
   ShaderLibrary.C 
   ShaderDialog.C 
   Lebedev.C 
   GLSLmath.C 
   GridInfoDialog.C 
   SurfaceAnimatorDialog.C 
   BoundingBoxDialog.C 
#  glloader.cpp 
   LocalConnectionThread.C
   QMsgBox/QMsgBox.C
)

set(IQmol_FORMS
   HelpBrowser.ui  PreferencesBrowser.ui  ToolBar.ui 
   AboutDialog.ui  MeshConfigurator.ui  BackgroundConfigurator.ui 
   MoleculeConfigurator.ui  SymmetryToleranceDialog.ui 
   ProgressDialog.ui  SurfaceConfigurator.ui CubeDataConfigurator.ui 
   InfoConfigurator.ui  FrequenciesConfigurator.ui 
   MolecularOrbitalsConfigurator.ui  ConformerListConfigurator.ui 
   FileConfigurator.ui  DipoleConfigurator.ui  GradientDialog.ui  
   VectorConstraintConfigurator.ui  ScalarConstraintConfigurator.ui 
   LogMessageDialog.ui  SetPasswordDialog.ui  GetVaultKeyDialog.ui 
   ServerListDialog.ui   
   FragmentTable.ui   
   ProcessMonitor.ui  SSHFileConfigurator.ui   
   ServerDialog.ui  ServerOptionsDialog.ui  EFPFragmentListConfigurator.ui 
   ServerQueueDialog.ui  ShaderDialog.ui  SurfaceAnimatorDialog.ui  GridInfoDialog.ui 
   BoundingBoxDialog.ui
)

if(APPLE)
    set(IQmol_FORMS ${IQmol_FORMS} PeriodicTableMac.ui)
else(APPLE)
    set(IQmol_FORMS ${IQmol_FORMS} PeriodicTable.ui)
endif(APPLE)

set(IQmol_RESOURCES
    IQmol.qrc
)
   
set(IQmol_Data_SOURCES
    Data/Atom.C
    Data/AtomicProperty.C
    Data/Bank.C
    Data/Charge.C
    Data/Data.C
    Data/DataFactory.C
    Data/EfpFragment.C
    Data/EfpFragmentLibrary.C
    Data/File.C
    Data/Frequencies.C
    Data/Geometry.C
    Data/GeometricProperty.C
    Data/GridData.C
    Data/Molecule.C
    Data/MultipoleExpansion.C
    Data/RemSectionData.C
    Data/VibrationalMode.C
)

set(IQmol_GL2PS_SOURCES
    GL2PS/gl2ps.C
)

set_property(SOURCE ${IQmol_GL2PS_SOURCES} APPEND PROPERTY
    COMPILE_DEFINITIONS GL2PS)

set(IQmol_Parser_HEADERS
    Parser/ParseFile.h
)

set(IQmol_Parser_SOURCES
    Parser/Parser.C
    Parser/ParseFile.C
    Parser/CartesianCoordinatesParser.C
    Parser/EfpFragmentParser2.C
    Parser/ExternalChargesParser2.C
    Parser/IQmolParser.C
    Parser/QChemInputParser.C
    Parser/QChemOutputParser.C
    Parser/QChemPlotParser.C
    Parser/XyzParser.C
    Parser/ZMatrixCoordinatesParser.C
)

set(IQmol_QCPRotation_SOURCES
    QCPRotation/qcprot.C
    QCPRotation/Align.C
)

set(IQmol_QsLog_SOURCES
    QsLog/QsLogDest.cpp
    QsLog/QsLog.cpp
    QsLog/QsDebugOutput.cpp
)

set_property(SOURCE ${IQmol_QsLog_SOURCES} APPEND PROPERTY
    COMPILE_DEFINITIONS QS_LOG_LINE_NUMBERS)

set(IQmol_QUI_HEADERS
   QUI/OptionDatabaseForm.h
   QUI/OptionEditors.h
   QUI/InputDialog.h
   QUI/QtNode.h
   QUI/FileDisplay.h
   QUI/GeometryConstraint.h
   QUI/FindDialog.h
   QUI/QuiMolecule.h
)

set(IQmol_QUI_SOURCES
   QUI/OptionDatabaseForm.C
   QUI/Option.C
   QUI/OptionDatabase.C
   QUI/OptionEditors.C
   QUI/Conditions.C
   QUI/Actions.C
   QUI/InputDialogLogic.C
   QUI/Job.C
   QUI/Qui.C
   QUI/FileDisplay.C
   QUI/KeywordSection.C
   QUI/ReadInput.C
   QUI/RemSection.C
   QUI/MoleculeSection.C
   QUI/InputDialog.C
   QUI/GeometryConstraint.C
   QUI/OptSection.C
   QUI/ExternalChargesSection.C
   QUI/LJParametersSection.C
   QUI/FindDialog.C
   QUI/KillProcess.C
   QUI/RunCommand.C
   QUI/GetParentProcessChain.C
   QUI/GetMatchingProcesses.C
   QUI/IQmolInterface.C
   QUI/QuiMolecule.C
   QUI/QuiReadFile.C
)

set(IQmol_QUI_FORMS
   QUI/OptionDatabaseForm.ui
   QUI/OptionListEditor.ui
   QUI/OptionNumberEditor.ui
   QUI/FileDisplay.ui
   QUI/QuiMainWindow.ui
   QUI/GeometryConstraintDialog.ui
   QUI/FindDialog.ui
)

set_property(SOURCE ${IQmol_QUI_SOURCES} APPEND PROPERTY
    COMPILE_DEFINITIONS QCHEM_UI)

qt4_wrap_cpp(IQmol_HEADERS_MOC
    ${IQmol_HEADERS} ${IQmol_Parser_HEADERS} ${IQmol_QUI_HEADERS})
qt4_wrap_ui(IQmol_FORMS_HEADERS
    ${IQmol_FORMS} ${IQmol_QUI_FORMS})
qt4_add_resources(IQmol_RESOURCES_RCC
    ${IQmol_RESOURCES})

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(Data GL2PS Parser QCPRotation QMsgBox QsLog)

add_executable(IQmol
    ${IQmol_SOURCES}
    ${IQmol_Data_SOURCES}
    ${IQmol_GL2PS_SOURCES}
    ${IQmol_Parser_SOURCES}
    ${IQmol_QCPRotation_SOURCES}
    ${IQmol_QsLog_SOURCES}
    ${IQmol_QUI_SOURCES}
    ${IQmol_HEADERS_MOC} ${IQmol_FORMS_HEADERS} ${IQmol_RESOURCES_RCC}
)

target_link_libraries(IQmol
    ${OPENBABEL2_LIBRARIES}
    ${QGLVIEWER_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${LIBSSH2_LIBRARY} ${OPENSSL_LIBRARIES}
    ${Boost_LIBRARIES} ${ZLIB_LIBRARIES}
)

