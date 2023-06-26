CONFIG -= qt

TEMPLATE = lib
DEFINES += MTRANSCODE_LIBRARY __STDC_CONSTANT_MACROS

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    moption.cpp \
    mtranscode.cpp

HEADERS += \
    mTranscode_global.h \
    moption.h \
    mtranscode.h

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

unix|win32: LIBS += -L$$PWD/../../3rd/ffmpeg/lib/ -lavcodec

INCLUDEPATH += $$PWD/../../3rd/ffmpeg/include
DEPENDPATH += $$PWD/../../3rd/ffmpeg/include

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/avcodec.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/libavcodec.a

unix|win32: LIBS += -L$$PWD/../../3rd/ffmpeg/lib/ -lavdevice

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/avdevice.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/libavdevice.a

unix|win32: LIBS += -L$$PWD/../../3rd/ffmpeg/lib/ -lavfilter

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/avfilter.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/libavfilter.a

unix|win32: LIBS += -L$$PWD/../../3rd/ffmpeg/lib/ -lavformat


win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/avformat.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/libavformat.a

unix|win32: LIBS += -L$$PWD/../../3rd/ffmpeg/lib/ -lavutil


win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/avutil.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/libavutil.a

unix|win32: LIBS += -L$$PWD/../../3rd/ffmpeg/lib/ -lswresample


win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/swresample.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/libswresample.a

unix|win32: LIBS += -L$$PWD/../../3rd/ffmpeg/lib/ -lswscale


win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/swscale.lib
else:unix|win32-g++: PRE_TARGETDEPS += $$PWD/../../3rd/ffmpeg/lib/libswscale.a
