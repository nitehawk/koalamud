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
					core \
          thread 

core {
	CONFIG += world cmd gui char
	SOURCES += main.cpp network.cpp database.cpp memory.cpp
	HEADERS += main.hxx network.hxx database.hxx event.hxx memory.hxx
}

world {
	SOURCES += room.cpp
	HEADERS += room.hxx
}

char {
	SOURCES += char.cpp playerchar.cpp
	HEADERS += char.hxx playerchar.hxx
}

cmd {
	SOURCES += cmd.cpp cmdtree.cpp comm.cpp
	HEADERS += cmd.hxx cmdtree.hxx comm.hxx
}

gui {
	FORMS += newnetworkportdlg.ui
	SOURCES += koalastatus.cpp
	HEADERS += koalastatus.h
}
