TARGET   = qtextedit
TEMPLATE = app

#
# Build configuration
#
CONFIG += qt thread warn_on

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
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-core/$${buildmode} -loaf-core-qt4

#
# OAF-OQL
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-oql/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-oql/$${buildmode} -loaf-oql-qt4

#
# OAF
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf/$${buildmode} -loaf-qt4

#
# OAF-GIT2
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-git2/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-git2/$${buildmode} -loaf-git2

#
# OAF-STD
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-std/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-std/$${buildmode} -loaf-std-qt4

#
# OAF-GUI
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-gui/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-gui/$${buildmode} -loaf-gui-qt4

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
