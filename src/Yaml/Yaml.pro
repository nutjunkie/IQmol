LIB = Yaml
CONFIG += lib
include(../common.pri)

INCLUDEPATH += yaml-cpp

SOURCES = \
   $$PWD/binary.cpp \
   $$PWD/convert.cpp \
   $$PWD/directives.cpp \
   $$PWD/emit.cpp \
   $$PWD/emitfromevents.cpp \
   $$PWD/emitter.cpp \
   $$PWD/emitterstate.cpp \
   $$PWD/emitterutils.cpp \
   $$PWD/exp.cpp \
   $$PWD/memory.cpp \
   $$PWD/node.cpp \
   $$PWD/node_data.cpp \
   $$PWD/nodebuilder.cpp \
   $$PWD/nodeevents.cpp \
   $$PWD/null.cpp \
   $$PWD/ostream_wrapper.cpp \
   $$PWD/parse.cpp \
   $$PWD/regex.cpp \
   $$PWD/scanner.cpp \
   $$PWD/scanscalar.cpp \
   $$PWD/scantag.cpp \
   $$PWD/scantoken.cpp \
   $$PWD/simplekey.cpp \
   $$PWD/singledocparser.cpp \
   $$PWD/stream.cpp \
   $$PWD/tag.cpp \
   $$PWD/yaml_parser.cpp \


HEADERS = \
   $$PWD/collectionstack.h \
   $$PWD/directives.h \
   $$PWD/emitterstate.h \
   $$PWD/emitterutils.h \
   $$PWD/exp.h \
   $$PWD/indentation.h \
   $$PWD/nodebuilder.h \
   $$PWD/nodeevents.h \
   $$PWD/ptr_stack.h \
   $$PWD/ptr_vector.h \
   $$PWD/regex.h \
   $$PWD/regeximpl.h \
   $$PWD/scanner.h \
   $$PWD/scanscalar.h \
   $$PWD/scantag.h \
   $$PWD/setting.h \
   $$PWD/singledocparser.h \
   $$PWD/stream.h \
   $$PWD/streamcharsource.h \
   $$PWD/stringsource.h \
   $$PWD/tag.h \
   $$PWD/token.h \

