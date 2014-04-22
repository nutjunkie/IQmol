#
# - Try to find OpenMesh
#
# Once done this will define:
#
#  OPENMESH_FOUND - system has OpenMesh
#  OPENMESH_INCLUDE_DIR - OpenMesh include directory
#  OPENMESH_LIBRARIES - Link these to use OpenMesh
#  OPENMESH_DEFINITIONS - Compiler switches required for using OpenMesh
#

find_path(OPENMESH_INCLUDE_DIR 
          NAMES OpenMesh/Core/IO/Options.hh
          PATHS /usr
                /usr/local
                ENV OPENMESHROOT 
          PATH_SUFFIXES include
         )
find_library(OPENMESH_CORE_LIB OpenMeshCored
    PATHS ENV OPENMESHROOT PATH_SUFFIXES OpenMesh lib/OpenMesh)
if(OPENMESH_CORE_LIB)
    set(OPENMESH_LIBRARIES ${OPENMESH_CORE_LIB})
endif(OPENMESH_CORE_LIB)
unset(OPENMESH_CORE_LIB)

IF(OPENMESH_INCLUDE_DIR AND OPENMESH_LIBRARIES)
   SET(OPENMESH_FOUND TRUE)
ENDIF(OPENMESH_INCLUDE_DIR AND OPENMESH_LIBRARIES)

IF(OPENMESH_FOUND)
  IF(NOT OpenMesh_FIND_QUIETLY)
    MESSAGE(STATUS "Found OpenMesh: ${OPENMESH_INCLUDE_DIR}")
  ENDIF(NOT OpenMesh_FIND_QUIETLY)
ELSE(OPENMESH_FOUND)
  IF(OpenMesh_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "Could not find OpenMesh")
  ENDIF(OpenMesh_FIND_REQUIRED)
ENDIF(OPENMESH_FOUND)

