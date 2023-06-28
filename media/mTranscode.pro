CONFIG -= qt

TEMPLATE = lib
DEFINES += MTRANSCODE_LIBRARY __STDC_CONSTANT_MACROS

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    internal/mtranscodeimpl.cpp \
    mtranscode.cpp

HEADERS += \
    internal/mtranscodeimpl.h \
    option/moption.h \
    option/moption_global.h \
    option/moption_log.h \
    mTranscode_global.h \
    mtranscode.h \
    mtranscode_error.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -L$$PWD/../3rd/ffmpeg/lib/ -llibavutil.dll -llibavformat.dll -llibavfilter.dll -llibavcodec.dll

INCLUDEPATH += $$PWD/../3rd/ffmpeg/include
DEPENDPATH += $$PWD/../3rd/ffmpeg/include
