TARGET   = oaf
TEMPLATE = lib

#
# Build configuration
#
CONFIG += qt thread warn_on

#
# Настройка динамической линковки под Windows
#
win32:CONFIG  += dll
win32:DEFINES += OAF_LIBRARY

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
# Настройка каталогов размещения собранных файлов
#
win32 {
	isEmpty(LIBRARY_INSTALL_PATH):LIBRARY_INSTALL_PATH = /bin
}
else {
	isEmpty(LIBRARY_INSTALL_PATH):LIBRARY_INSTALL_PATH = /usr/lib/oaf
}

#
# Путь установки библиотеки
#
target.path = $${LIBRARY_INSTALL_PATH}

#
# Настройка инсталляции
#
INSTALLS += target

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
# Дополнительный путь поиска заголовочных файлов
#
INCLUDEPATH += include

#
# Заголовочные файлы
#
HEADERS += \
	include/OAF/OafGlobal.h \
	include/OAF/OAF.h \
	src/CClassInfo.h \
	src/CFunctionFactory.h

#
# Исходные тексты
#
SOURCES += \
	src/OAF.cpp \
	src/CClassInfo.cpp \
	src/CFunctionFactory.cpp
