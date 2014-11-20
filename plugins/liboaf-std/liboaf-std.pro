TARGET   = oaf-std-1.0
TEMPLATE = lib

#
# Build configuration
#
CONFIG += qt thread warn_on plugin

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
	isEmpty(PLUGIN_INSTALL_PATH):PLUGIN_INSTALL_PATH = /bin/plugins
}
else {
	isEmpty(PLUGIN_INSTALL_PATH):PLUGIN_INSTALL_PATH = /usr/lib/oaf-qt4/plugins
}

#
# Путь установки библиотеки
#
target.path = $${PLUGIN_INSTALL_PATH}

#
# Установка OAF-файла
#
oaf.files = liboaf-std-1.0.oaf
oaf.path  = $${PLUGIN_INSTALL_PATH}

#
# Настройка инсталляции
#
INSTALLS += target oaf

#
# Устанавливаем oaf-файлы в каталог сборки
#
system($$QMAKE_MKDIR $$buildmode)
system($$QMAKE_COPY *.oaf $$buildmode)

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
# OAF-STD
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-std/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-std/$${buildmode} -loaf-std-qt4

#
# Подключаем информацию о версии libOAF
#
include ($${TOPSRCDIR}/liboaf_version.pri)

#
# Заголовочные файлы
#
HEADERS += \
	src/CCommon.h \
	src/CFactory.h \
	src/CMonikerSTD.h

#
# Исходные тексты
#
SOURCES += \
	src/CCommon.cpp \
	src/CFactory.cpp \
	src/CMonikerSTD.cpp \
	src/main.cpp
