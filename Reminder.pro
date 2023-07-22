QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/config.cpp \
    src/dialog_audio_settings.cpp \
    src/mainwindow.cpp \
    src/reminder.cpp \
    src/singleappservice.cpp

HEADERS += \
    src/audioconfig.h \
    src/config.h \
    src/dialog_audio_settings.h \
    src/mainwindow.h \
    src/reminder.h \
    src/reminder_data.h \
    src/singleappservice.h

FORMS += \
    ui/config.ui \
    ui/dialog_audio_settings.ui \
    ui/mainwindow.ui
    ui/config.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES += \
    winicon.rc

RC_FILE += \
    winicon.rc
