TARGET   = oaf-oql
TEMPLATE = lib

#
# Build configuration
#
CONFIG += qt thread warn_on c++11

#
# Настройка динамической линковки под Windows
#
win32:CONFIG  += dll
win32:DEFINES += OAFOQL_LIBRARY

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
# Для сборки под MSVC отключаем использование <unistd.h> в bison
#
win32:DEFINES += YY_NO_UNISTD_H

#
# Для сборки под MSVC нужно явно настроить параметры запуска bison и
# копирования полученных файлов
#
win32 {
	QMAKE_LEX              = flex
	QMAKE_YACC             = bison
	QMAKE_YACCFLAGS_MANGLE = -p OQLGram -b OQLGram
	QMAKE_YACC_HEADER      = ${QMAKE_FILE_BASE}.tab.h
	QMAKE_YACC_SOURCE      = ${QMAKE_FILE_BASE}.tab.c
	QMAKE_MOVE             = copy
}

#
# Для сборки с новой версией bison приходится извращаться, чтобы
# "обмануть" qmake, который упорно хочет сделать MOVE для *_HEADER
# и *_SOURCE
#
unix {
	QMAKE_LEX              = flex
	QMAKE_YACC             = bison
	QMAKE_YACCFLAGS_MANGLE = -b OQLgram --output=OQLgram_gen.cpp --defines=OQLgram_gen.h
	QMAKE_YACC_HEADER      = OQLgram_gen.h
	QMAKE_YACC_SOURCE      = OQLgram_gen.cpp
}

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
# Дополнительный путь поиска заголовочных файлов
#
INCLUDEPATH += include

#
# Заголовочные файлы
#
HEADERS += \
	include/OAF/OafOqlGlobal.h \
	include/OAF/CExpression.h \
	include/OAF/OQL.h

#
# Исходные тексты
#
SOURCES += \
	src/CExpression.cpp

#
# Исходный код сканера и парсера
#
LEXSOURCES  += src/OQLscan.l
YACCSOURCES += src/OQLgram.y
