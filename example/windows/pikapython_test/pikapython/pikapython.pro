QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += WIN32_LEAN_AND_MEAN
DEFINES += _CRT_NONSTDC_NO_DEPRECATE
DEFINES += _CRT_SECURE_NO_WARNINGS
DEFINES += HAVE_STRUCT_TIMESPEC
DEFINES += _WINSOCK_DEPRECATED_NO_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += PIKA_CONFIG_ENABLE
DEFINES += PIKA_PLATFORM_NO_WEAK

SOURCES += \
    PikaPlatform_win.c \
    main.c

HEADERS += \
    pika_config.h

HEADERS += $$files(../../lib/pthread/*.h, true)
HEADERS += $$files(../../lib/libserialport/*.h, true)
HEADERS += $$files(../../../lib/pikapython/*.h, true)
HEADERS += $$files(../../../../src/agile_modbus/*.h, true)
HEADERS += $$files(../../../../src/modbus_rt/*.h, true)
HEADERS += $$files(../../../../src/platform/linux_win/*.h, true)
HEADERS += $$files(../../../../src/slave_util/*.h, true)

SOURCES += $$files(../../../lib/pikapython/*.c, true)
SOURCES += $$files(../../../../src/agile_modbus/*.c, true)
SOURCES += $$files(../../../../src/modbus_rt/*.c, true)
SOURCES += $$files(../../../../src/platform/linux_win/*.c, true)
SOURCES += $$files(../../../../src/slave_util/*.c, true)

# 添加头文件搜索路径
INCLUDEPATH +=  $$PWD/../../../../src/agile_modbus \
                $$PWD/../../../../src/modbus_rt \
                $$PWD/../../../../src/platform/linux_win \
                $$PWD/../../../../src/slave_util \
                $$PWD/../../../lib/pikapython/pikascript-core \
                $$PWD/../../../lib/pikapython/pikascript-api  \
                $$PWD/../../../lib/pikapython/pikascript-lib/PikaStdLib  \
                $$PWD/../../../lib/pikapython/pikascript-lib/json  \
                $$PWD/../../../lib/pikapython/pikascript-lib/os  \
                $$PWD/../../../lib/pikapython/pikascript-lib/socket  \


TRANSLATIONS += \
    pikapython_zh_CN.ts

# MSVC Compiler - Disable warning C4244
# QMAKE_CFLAGS += /wd4087
win32-msvc* {
    QMAKE_CFLAGS += /wd4003
    QMAKE_CFLAGS += /wd4018
    QMAKE_CFLAGS += /wd4113
    QMAKE_CFLAGS += /wd4133
    QMAKE_CFLAGS += /wd4200
    QMAKE_CFLAGS += /wd4244
    QMAKE_CFLAGS += /wd4267
    QMAKE_CFLAGS += /wd4273
    QMAKE_CFLAGS += /wd4819
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: LIBS += -L$$PWD/../../lib/ -llibserialport

INCLUDEPATH += $$PWD/../../lib/libserialport
DEPENDPATH += $$PWD/../../lib/libserialport

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../lib/libserialport.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../../lib/liblibserialport.a

win32: LIBS += -L$$PWD/../../lib/ -lpthreadVC2

INCLUDEPATH += $$PWD/../../lib/pthread
DEPENDPATH += $$PWD/../../lib/pthread

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/../../lib/pthreadVC2.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/../../lib/libpthreadVC2.a
