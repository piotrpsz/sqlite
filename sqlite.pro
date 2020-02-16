TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
LIBS += -L/usr/lib/x86_64-linux-gnu/ -lsqlite3
//INCLUDEPATH += /usr/include/

# -L/path/to -lpsapi

SOURCES += \
        Account.cpp \
        SQLite/Field.cpp \
        SQLite/SQLite.cpp \
        SQLite/Statement.cpp \
        main.cpp

HEADERS += \
   Account.h \
   SQLite/Field.h \
   SQLite/SQLite.h \
   SQLite/Statement.h
