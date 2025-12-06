QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    BFS.cpp \
    BoardEditor.cpp \
    BoardEngine.cpp \
    BoardWidget.cpp \
    GameWindow.cpp \
    Queue.cpp \
    RandomBoard.cpp \
    StatsFormatter.cpp \
    adj_list.cpp \
    main.cpp \
    page1.cpp

HEADERS += \
    BFS.h \
    BoardEditor.h \
    BoardEngine.h \
    BoardWidget.h \
    GameWindow.h \
    Queue.h \
    RandomBoard.h \
    StatsFormatter.h \
    adj_list.h \
    page1.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
