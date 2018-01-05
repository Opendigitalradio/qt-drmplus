#-------------------------------------------------
#
# Project created by QtCreator 2017-08-13T17:33:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets charts

TARGET = qt-drmplus
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


CONFIG          += console
#QMAKE_CFLAGS    +=  -flto -ffast-math
#QMAKE_CXXFLAGS  +=  -flto -ffast-math
#QMAKE_LFLAGS    +=  -flto

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    MultColorLED.cpp \
    GraphDialog.cpp \
    InOutDialog.cpp \
    output/audio-base.cpp \
    output/audiosink.cpp \
    output/fir-filters.cpp \
    output/newconverter.cpp \
    DRMWorker.cpp \
    devices/virtual-input.cpp \
    devices/rawfiles/rawfiles.cpp \
    devices/wavfiles/wavfiles.cpp \
    TechInfoDialog.cpp \
    devices/resample.c

HEADERS += \
    mainwindow.h \
    MultColorLED.h \
    GraphDialog.h \
    InOutDialog.h \
    output/audio-base.h \
    output/audiosink.h \
    output/fir-filters.h \
    output/newconverter.h \
    drm-constants.h \
    ringbuffer.h \
    DRMWorker.h \
    devices/virtual-input.h \
    devices/rawfiles/rawfiles.h \
    devices/wavfiles/wavfiles.h \
    TechInfoDialog.h \
    devices/speex_resampler.h



DEPENDPATH += . \
              ./devices \
              ./devices/rawfiles \
              ./devices/wavfiles \


INCLUDEPATH += . \
              ./devices \
              ./devices/rawfiles \
              ./devices/wavfiles

INCLUDEPATH    += output

INCLUDEPATH    += ../include

#static lib

unix {
#INCLUDEPATH    += /usr/local/include
#LIBS            += -lfftw3f  -lusb-1.0  #
LIBS            += -ldl
LIBS            += -lportaudio
#LIBS            += -lz
LIBS            += -lsndfile
LIBS            += -lsamplerate
#LIBS            += -lfaad2_drm
LIBS            += -L./ -lfaad2_drm

CONFIG          += dabstick
#CONFIG          += sdrplay
#CONFIG          += rtl_tcp
#CONFIG          += airspy

CONFIG          += embed_libdrmplus
}

# an attempt to have it run under W32
win32 {
# includes in mingw differ from the includes in fedora linux
#INCLUDEPATH += /usr/i686-w64-mingw32/sys-root/mingw/include
LIBS            += -L/usr/i686-w64-mingw32/sys-root/mingw/lib
#LIBS           += -lfftw3f
LIBS            += -lportaudio
LIBS            += -lsndfile
LIBS            += -lsamplerate
LIBS            += -lole32
LIBS            += -lwinpthread
LIBS            += -lwinmm
LIBS            += -lstdc++
LIBS            += -lws2_32
LIBS            += -lfaad2_drm
#LIBS           += -lusb-1.0
#LIBS           += -lz

CONFIG          += extio
#CONFIG         += airspy
CONFIG         += rtl_tcp
CONFIG          += dabstick
#CONFIG         += sdrplay
}


FORMS += \
    DRMMainWindow.ui \
    GraphDialog.ui \
    InOutDialog.ui \
    TechInfoDialog.ui \
    StationInfo.ui

FORMS += devices/filereader-widget.ui

DISTFILES += \
    icons/media-repeat-alt.png

RESOURCES += \
    icons.qrc




#	libraries
embed_libdrmplus {
        SOURCES		+= ../src/audiotext.c \
                           ../src/drmplus.c \
                           ../src/midiviterbi.c \
                           ../src/parser_fac.c \
                           ../src/parser_msc.c \
                           ../src/parser_sdc.c \
                           ../src/parser_str.c \
                           ../src/rom_tables.c \
                           ../src/sig_proc.c \
                           ../src/sig_sync.c \
                           ../src/utils.c
        INCLUDEPATH	+= ../src/
        HEADERS += \
                           ../src/audiotext.h    \
                           ../src/drmplus_internal.h \
                           ../src/midiviterbi.h
        LIBS            += -lfftw3 -lm

} else {
        LIBS            += ../src/.libs/libdrmplus.a -lfftw3 -lm

}


#	devices
#
#	dabstick
dabstick {
        DEFINES		+= HAVE_RTLSDR
        DEPENDPATH	+= devices/rtlsdr-handler
        INCLUDEPATH	+= devices/rtlsdr-handler
        HEADERS		+= devices/rtlsdr-handler/rtlsdr-handler.h \
                           devices/rtlsdr-handler/rtl-dongleselect.h
        SOURCES		+= devices/rtlsdr-handler/rtlsdr-handler.cpp \
                           devices/rtlsdr-handler/rtl-dongleselect.cpp
        FORMS		+= devices/rtlsdr-handler/rtlsdr-widget.ui
}

#	extio dependencies, windows only
#
extio {
        DEFINES		+= HAVE_EXTIO
        INCLUDEPATH	+= devices/extio-handler
        HEADERS		+= devices/extio-handler/extio-handler.h \
                           devices/extio-handler/common-readers.h \
                           devices/extio-handler/virtual-reader.h
        SOURCES		+= devices/extio-handler/extio-handler.cpp \
                           devices/extio-handler/common-readers.cpp \
                           devices/extio-handler/virtual-reader.cpp
}

#
rtl_tcp {
        DEFINES		+= HAVE_RTL_TCP
        QT		+= network
        INCLUDEPATH	+= devices/rtl_tcp
        HEADERS		+= devices/rtl_tcp/rtl_tcp_client.h
        SOURCES		+= devices/rtl_tcp/rtl_tcp_client.cpp
        FORMS		+= devices/rtl_tcp/rtl_tcp-widget.ui
}
