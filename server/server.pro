QT += core network sql

CONFIG += console c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = rental_spaces_server

SOURCES += \
    main.cpp \
    Network/mytcpserver.cpp \
    Database/databasemanager.cpp \
    Handlers/requesthandler.cpp

HEADERS += \
    Network/mytcpserver.h \
    Database/databasemanager.h \
    Handlers/requesthandler.h
