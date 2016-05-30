LIB = OpenMesh
CONFIG += lib
include(../common.pri)
include(qmake/all.include)

Subdirs()

addSubdirs( src/OpenMesh/Core )
addSubdirs( src/OpenMesh/Tools , src/OpenMesh/Core )
