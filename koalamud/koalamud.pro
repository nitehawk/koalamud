TARGET = koalamud
DESTDIR = ../bin
DEFINES += _GNU_SOURCE
UI_DIR = .uic
MOC_DIR = .moc
OBJECTS_DIR = .obj
TEMPLATE = app 
CONFIG += debug \
          warn_on \
          qt \
          thread 
SOURCES += main.cpp \
           koalastatus.cpp \
           network.cpp \
           char.cpp \
           playerchar.cpp \
					 cmd.cpp
HEADERS += main.hxx \
           koalastatus.h \
           network.hxx \
           char.hxx \
           playerchar.hxx \
					 cmd.hxx
FORMS += newnetworkportdlg.ui 
