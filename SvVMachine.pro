#
#  Проект     "Скриптовый язык reduced c++ (rc++) v6"
#  Подпроект  "Пико-компилятор"
#  Автор
#    Alexander Sibilev
#  Интернет
#    www.rc.saliLab.ru - домашний сайт проекта
#    www.saliLab.ru
#    www.saliLab.com
#
#  Описание
#    Виртуальная машина для исполнения байт-кода. Проект предназначен
#    для проверки на предмет ошибок компиляции
#
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


DISTFILES +=

HEADERS += \
  SvVMachine/Sv6Plc.h \
  SvVMachine/Sv6Sys.h \
  SvVMachine/SvVMachine.h \
  SvVMachine/SvVmByteCode.h \
  SvVMachine/SvVmCodeHeader.h \
  SvVMachine/SvVmTypes.h \
  SvVMachine/SvVmUsbInterface.h \
  SvVMachine/SvVmUtils.h \
  SvVMachine/SvVmVpu.h \
  SvVMachine/SvVmVpuState.h

SOURCES += \
  SvVMachine/SvVMachine.cpp \
  SvVMachineMain.cpp


