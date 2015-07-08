QT += network

CONFIG += staticlib

win32:CONFIG += console

TEMPLATE = lib

TARGET = CarControllerInterProcessSignalPropagator
DESTDIR = ./../lib
DEPENDPATH += .
INCLUDEPATH += . \
                ./../../../InterProcessSignalPropagatorLibrary

# Input
HEADERS += CarControllerInterProcessSignalPropagator.h
SOURCES += CarControllerInterProcessSignalPropagator.cpp
