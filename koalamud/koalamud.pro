TARGET = koalamud
DESTDIR = ../bin
DEFINES += _GNU_SOURCE _POSIX REENTRANT
UI_DIR = .uic
MOC_DIR = .moc
OBJECTS_DIR = .obj
TEMPLATE = app 
LIBS += -lZThread
CONFIG += debug \
          warn_on \
          qt \
          thread 
SOURCES += main.cpp \
           koalastatus.cpp \
           network.cpp \
           char.cpp \
           playerchar.cpp \
					 cmd.cpp \
					 database.cpp \
					 comm.cpp
HEADERS += main.hxx \
           koalastatus.h \
           network.hxx \
           char.hxx \
           playerchar.hxx \
					 cmd.hxx \
					 comm.hxx \
					 database.hxx \
					 event.hxx
FORMS += newnetworkportdlg.ui 
