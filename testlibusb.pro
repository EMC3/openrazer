QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
    driver/razerchromacommon.cpp \
    driver/razercommon.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target



win32{
    LIBS += -LD:\QT\mlibs\build-mlibs-5_11_0_x64-Release\release
    LIBS += -LD:\mingw-w64\x86_64-7.3.0-posix-seh-rt_v5-rev0\install_root\lib
    INCLUDEPATH += D:\QT\mlibs\mlibs
    INCLUDEPATH += D:\mingw-w64\x86_64-7.3.0-posix-seh-rt_v5-rev0\install_root\include


    LIBS +=  -lusb-1.0.dll -lyaml-cpp.dll -lmlibs_qt -lyaml-cpp.dll -lcrypto -lws2_32 -lgdi32 -lWinmm -lz
}

HEADERS += \
    driver/razerchromacommon.h \
    driver/razercommon.h \
    driver/razerkbd_driver.h
