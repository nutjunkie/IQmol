#! /bin/sh

GIT_VERSION=`git describe --always --tags`
IQMOL_VERSION=`git describe --abbrev=0`
IQMOL_YEAR=`date +%Y`
IQMOL_DATE=`date `

if [ -e version.h ]; then
   rm version.h
fi
echo "// Version file generated on \"$IQMOL_DATE\"" > version.h
echo "#define IQMOL_YEAR \"$IQMOL_YEAR\"" >> version.h
echo "#define IQMOL_VERSION \"$IQMOL_VERSION\"" >> version.h
echo "#define GIT_VERSION \"$GIT_VERSION\"" >> version.h
