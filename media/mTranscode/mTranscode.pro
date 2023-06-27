CONFIG -= qt

TEMPLATE = lib
DEFINES += MTRANSCODE_LIBRARY __STDC_CONSTANT_MACROS

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../internal/mutils.c \
    ../internal/objpool.c \
    ../internal/sync_queue.c \
    mtranscode.cpp \
    mtranscodeinternal.c

HEADERS += \
    ../internal/mutils.h \
    ../internal/objpool.h \
    ../internal/sync_queue.h \
    ../option/moption.h \
    ../option/moptlog.h \
    mTranscode_global.h \
    mtranscode.h \
    mtranscode_error.h \
    mtranscodeinternal.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -L$$PWD/../../3rd/ffmpeg/lib/ -llibavutil.dll -llibavformat.dll -llibavfilter.dll -llibavcodec.dll

INCLUDEPATH += $$PWD/../../3rd/ffmpeg/include
DEPENDPATH += $$PWD/../../3rd/ffmpeg/include
