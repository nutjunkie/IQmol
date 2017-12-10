# CPack configuration for development build 
# must set GIT_TAG=`git describe` to get package version *** 

### Get Git tag 
execute_process(COMMAND git describe --always --tags OUTPUT_VARIABLE GIT_TAG OUTPUT_STRIP_TRAILING_WHITESPACE)

### Basic
string(TIMESTAMP SYSTEM_DATE "%Y%m%d")
set(CPACK_PACKAGE_NAME "IQmol")
#set(CPACK_PACKAGE_VERSION_MAJOR "2")
#set(CPACK_PACKAGE_VERSION_MINOR "10")
#set(CPACK_PACKAGE_VERSION_PATCH "0")
#set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
string(REGEX REPLACE "^v" "" tag_string "${GIT_TAG}")
string(REGEX REPLACE "-.*" "" CPACK_PACKAGE_VERSION "${tag_string}")
string(REGEX REPLACE "${CPACK_PACKAGE_VERSION}-" "" extra_string "${tag_string}")
string(REGEX REPLACE "-.*" "" change_number "${extra_string}")
string(REGEX REPLACE "${change_number}-" "" git_hash "${extra_string}")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}.${change_number}")
set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A molecule editor and visualization package (development build)")
set(CPACK_PACKAGE_VENDOR "IQmol.org")
set(CPACK_PACKAGE_CONTACT "Andrew Gilbert")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${SYSTEM_DATE}")

### Generator
if(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
elseif(WIN32)
    set(CPACK_GENERATOR "WIX")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

set(IQMOL_SOURCE_DIR $ENV{PWD})
message("IQmol source directory: ${IQMOL_SOURCE_DIR}")

### Files
# cpack_installed_directories "/full/path;subdir"
# => subdir/files_in_full_path
set(CPACK_RESOURCE_FILE_LICENSE "${IQMOL_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${IQMOL_SOURCE_DIR}/README")
if(APPLE)
    set(CPACK_INSTALLED_DIRECTORIES "${IQMOL_SOURCE_DIR}/dist;.") 
    set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}-${git_hash}") # ex. 2.10.0.49-g7116ad0
elseif(WIN32)
    set(CPACK_INSTALLED_DIRECTORIES "${IQMOL_SOURCE_DIR}/dist;.") 
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_FILE_NAME}-windows-installer")
    set(CPACK_RESOURCE_FILE_LICENSE "${IQMOL_SOURCE_DIR}/LICENSE.txt")
else()
    set(CPACK_INSTALLED_DIRECTORIES "${IQMOL_SOURCE_DIR}/dist;usr") 
    set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}-${git_hash}") # ex. 2.10.0.49-g7116ad0
endif()

# cpack_packaing_install_prefix "prefix"
# tgz => cpack_package_file_name/prefix/subdir/files_in_full_path
# nsis => 
#set(CPACK_PACKAGING_INSTALL_PREFIX "/tmp/cpacktest")

## macOS
set(CPACK_DMG_VOLUME_NAME "${CPACK_PACKAGE_VERSION}")
set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK "ON")

### Windows
set(CPACK_PACKAGE_EXECUTABLES "iqmol;IQmol") # link in Start Menu
set(CPACK_CREATE_DESKTOP_LINKS "iqmol;IQmol")
# WiX
set(CPACK_WIX_SIZEOF_VOID_P 4)	# 32-bit
set(CPACK_WIX_PROGRAM_MENU_FOLDER "${CPACK_PACKAGE_INSTALL_DIRECTORY}")
set(CPACK_WIX_PRODUCT_ICON "${IQMOL_SOURCE_DIR}/src/Main/resources/IQmol.ico") 
set(CPACK_WIX_UI_BANNER "${IQMOL_SOURCE_DIR}/src/Main/resources/Installer-header.bmp")
set(CPACK_WIX_UI_DIALOG "${IQMOL_SOURCE_DIR}/src/Main/resources/Installer-lhs.bmp")

### Debian
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andrew Gilbert")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt5-default")

# ???
#set(CPACK_STRIP_FILES "bin/iqmol;IQmol")
