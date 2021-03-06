QT       += core gui testlib

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = litehtml-qt
TEMPLATE = app

MOC_DIR         = temp/moc
RCC_DIR         = temp/rcc
UI_DIR          = temp/ui
OBJECTS_DIR     = temp/obj
DESTDIR         = $$PWD/bin

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++14

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/fontcache.cpp \
    pathqt.cpp \
    graphicscontext.cpp \
    color.cpp \
    affinetransform.cpp \
    transformationmatrix.cpp \
    floatrect.cpp \
    floatpoint.cpp \
    intpoint.cpp \
    intsize.cpp \
    floatsize.cpp \
    intrect.cpp \
    floatquad.cpp \
    floatpoint3d.cpp \
    gradient.cpp \
    common.cpp \
    image.cpp \
    imagesource.cpp \
    imagedecoderqt.cpp \
    imagedecoder.cpp \# \
    stillimageqt.cpp \
    bitmapimage.cpp \
    imageobserver.cpp \
    mimetyperegistry.cpp \
    mimetyperegistryqt.cpp \
    sharedbuffer.cpp \
    purgeablebuffer.cpp \
    rgba32bufferqt.cpp \
    styleimage.cpp \
    contextshadow.cpp \
    shadowdata.cpp
    #pngimagedecoder.cpp
    # \
    #vector.cpp

HEADERS += \
        include/litehtml.h \
    src/mainwindow.h \
    src/fontcache.h \
    pathqt.h \
    graphicscontext.h \
    color.h \
    affinetransform.h \
    common.h \
    transformationmatrix.h \
    floatrect.h \
    floatpoint.h \
    intpoint.h \
    intsize.h \
    floatsize.h \
    intrect.h \
    floatquad.h \
    floatpoint3d.h \
    gradient.h \
    image.h \
    imagesource.h \
    imagedecoderqt.h \
    imagedecoder.h \# \
    stillimageqt.h \
    bitmapimage.h \
    imageobserver.h \
    mimetyperegistry.h \
    mimetyperegistryqt.h \
    sharedbuffer.h \
    purgeablebuffer.h \
    rgba32bufferqt.h \
    styleimage.h \
    contextshadow.h \
    shadowdata.h
    #pngimagedecoder.h
    # \
    #vector.h \
    #vectortraits.h

INCLUDEPATH     += $$PWD
INCLUDEPATH     += $$PWD/src
INCLUDEPATH     += $$PWD/include

include(src/gumbo/gumbo.pri)
include(src/litehtml/litehtml.pri)
include(containers/qt5/container-qt5.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    src/mainwindow.ui

RESOURCES += \
    res.qrc

