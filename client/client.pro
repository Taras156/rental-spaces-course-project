QT += core gui widgets network

CONFIG += c++17

TEMPLATE = app
TARGET = rental_spaces_client

SOURCES += \
    main.cpp \
    Network/singletonclient.cpp \
    Controllers/authcontroller.cpp \
    Controllers/admincontroller.cpp \
    Controllers/clientcontroller.cpp \
    Models/user.cpp \
    Models/client.cpp \
    Models/retailspace.cpp \
    Models/contractinfo.cpp \
    Models/paymentinfo.cpp \
    Styles/thememanager.cpp \
    Styles/themetoggleswitch.cpp \
    Views/authwindow.cpp \
    Views/adminwindow.cpp \
    Views/admintablewindow.cpp \
    Views/financereportwindow.cpp \
    Views/clientwindow.cpp

HEADERS += \
    Network/singletonclient.h \
    Controllers/authcontroller.h \
    Controllers/admincontroller.h \
    Controllers/clientcontroller.h \
    Models/user.h \
    Models/client.h \
    Models/retailspace.h \
    Models/contractinfo.h \
    Models/paymentinfo.h \
    Styles/thememanager.h \
    Styles/themetoggleswitch.h \
    Views/authwindow.h \
    Views/adminwindow.h \
    Views/admintablewindow.h \
    Views/financereportwindow.h \
    Views/clientwindow.h
