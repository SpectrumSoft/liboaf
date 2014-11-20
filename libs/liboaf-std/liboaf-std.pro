TARGET   = oaf-std-qt4
TEMPLATE = lib

#
# Build configuration
#
CONFIG += qt thread warn_on

#
# Настройка динамической линковки под Windows
#
win32:CONFIG  += dll
win32:DEFINES += OAFSTD_LIBRARY

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
# Явно подключаем webkit - это нужно делать в любой версии Qt
#
QT += webkit xml

#
# OAF-ROOT
#
TOPSRCDIR    = ../..
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
INCLUDEPATH  += $${TOPSRCDIR}/libs/liboaf-git2/include
LIBS         += -L$${TOPSRCDIR}/libs/liboaf-git2/$${buildmode} -loaf-git2

#
# Дополнительный путь поиска заголовочных файлов
#
INCLUDEPATH += include

#
# Заголовочные файлы
#
HEADERS += \
	include/OAF/OafStdGlobal.h \
	include/OAF/CTreeNode.h \
	include/OAF/CRecentFiles.h \
	include/OAF/Helpers.h \
	include/OAF/HelpersTXT.h \
	include/OAF/diff_match_patch.h \
	include/OAF/HtmlHelpers.h \
	include/OAF/ExchangeHelpers.h \
	include/OAF/MimeHelpers.h \
	include/OAF/IconHelpers.h \
	include/OAF/CProgressWatcher.h \
	include/OAF/CGitUrl.h \
	include/OAF/Consts.h

#
# Исходные тексты
#
SOURCES += \
	src/CTreeNode.cpp \
	src/CRecentFiles.cpp \
	src/Helpers.cpp \
	src/HelpersTXT.cpp \
	src/diff_match_patch.cpp \
	src/HtmlHelpers.cpp \
	src/ExchangeHelpers.cpp \
	src/MimeHelpers.cpp \
	src/IconHelpers.cpp \
	src/CProgressWatcher.cpp \
	src/CGitUrl.cpp
