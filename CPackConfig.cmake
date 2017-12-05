if(APPLE)
    set(CPACK_GENERATOR "Bundle")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
elseif(WIN32)
    set(CPACK_GENERATOR "NSIS")
else()
    set(CPACK_GENERATOR "TGZ")
endif()

# Basic
set(CPACK_PACKAGE_NAME "IQmol")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "9")
set(CPACK_PACKAGE_VERSION_PATCH "3")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A molecule editor and visualization package")
set(CPACK_PACKAGE_VENDOR "Andrew Gilbert")
set(CPACK_PACKAGE_CONTACT "http://iqmol.org")

# cpack_installed_directories "/full/path;subdir"
# => subdir/files_in_full_path
set(CPACK_INSTALLED_DIRECTORIES "/home/wesley/work/iqmol/IQmol/deploy;usr") 

# cpack_packaing_install_prefix "prefix"
# tgz => cpack_package_file_name/prefix/subdir/files_in_full_path
set(CPACK_PACKAGING_INSTALL_PREFIX "/tmp/cpacktest")

# Windows
set(CPACK_PACKAGE_DESCRIPTION_FILE "./README")
set(CPACK_RESOURCE_FILE_LICENSE "./LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "IQmol")
set(CPACK_PACKAGE_EXECUTABLES "bin/iqmol;IQmol")

# ???
set(CPACK_STRIP_FILES "bin/iqmol;IQmol")

# Debian
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andrew Gilbert")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "qt5-default")
