TARGET   = oaf-core-qt4
TEMPLATE = lib

#
# Build configuration
#
CONFIG += qt thread warn_on

#
# Настройка динамической линковки под Windows
#
win32:CONFIG  += dll
win32:DEFINES += OAFCORE_LIBRARY

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
	isEmpty(LIBRARY_INSTALL_PATH):LIBRARY_INSTALL_PATH = /usr/lib/oaf-qt4
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
# OAF-ROOT
#
TOPSRCDIR    = ../..
INCLUDEPATH += $${TOPSRCDIR}

#
# Дополнительный путь поиска заголовочных файлов
#
INCLUDEPATH += include

#
# Заголовочные файлы
#
HEADERS += \
	include/OAF/OafCoreGlobal.h \
	include/OAF/CNotifySource.h \
	include/OAF/CUnknown.h \
	include/OAF/CPropertyBag.h \
	include/OAF/CMoniker.h \
	include/OAF/StreamUtils.h \
	include/OAF/TypeUtils.h \
	include/OAF/Utils.h

SOURCES += \
	src/CNotifySource.cpp \
	src/CUnknown.cpp \
	src/CPropertyBag.cpp \
	src/CMoniker.cpp \
	src/StreamUtils.cpp \
	src/TypeUtils.cpp \
	src/Utils.cpp
