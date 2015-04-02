TARGET   = qtextedit
TEMPLATE = app

#
# Build configuration
#
CONFIG += qt thread warn_on

#
# Use standard widgets library in Qt5
#
greaterThan(QT_MAJOR_VERSION, 4)
{
	QT += widgets
}

#
# Данный набор флагов необходим для корректной работы механизма
# RTTI между загружаемыми внешними библиотеками под UNIX. Данный
# флаг должен использоваться при линковке ВСЕХ компонент
# системы: библиотек, плагинов и приложений.
#
unix:QMAKE_LFLAGS += -Wl,-E

#
# Режим сборки (по умолчанию - release)
#
buildmode = release
CONFIG(debug, debug|release):buildmode = debug

#
# Настройка директорий сборки отдельно для каждого из режимов
#
DESTDIR     = $${buildmode}
UI_DIR      = $${buildmode}
OBJECTS_DIR = $${buildmode}
MOC_DIR     = $${buildmode}

#
# Debug Windows output
#
win32:CONFIG(debug):CONFIG += console

#
# ROOT
#
TOPSRCDIR    = ../../
INCLUDEPATH += $${TOPSRCDIR}

#
# OAF-CORE
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-core/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-core/$${buildmode} -loaf-core

#
# OAF-OQL
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-oql/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-oql/$${buildmode} -loaf-oql

#
# OAF
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf/$${buildmode} -loaf

#
# OAF-GIT2
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-git2/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-git2/$${buildmode} -loaf-git2

#
# OAF-STD
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-std/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-std/$${buildmode} -loaf-std

#
# OAF-GUI
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-gui/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-gui/$${buildmode} -loaf-gui

#
# Заголовочные файлы
#
HEADERS += \
	src/CMainWindow.h

#
# Исходные тексты
#
SOURCES += \
	src/CMainWindow.cpp \
	src/main.cpp
