TARGET   = oaf-gui-qt4
TEMPLATE = lib

#
# Build configuration
#
CONFIG += qt thread warn_on

#
# Настройка динамической линковки под Windows
#
win32:CONFIG  += dll
win32:DEFINES += OAFGUI_LIBRARY

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
# OAF-STD
#
INCLUDEPATH += $${TOPSRCDIR}/libs/liboaf-std/include
LIBS        += -L$${TOPSRCDIR}/libs/liboaf-std/$${buildmode} -loaf-std-qt4

#
# Дополнительный путь поиска заголовочных файлов
#
INCLUDEPATH += include

#
# Заголовочные файлы
#
HEADERS += \
	include/OAF/OafGuiGlobal.h \
	include/OAF/CLineEdit.h \
	include/OAF/CTabWidget.h \
	include/OAF/CUIContainer.h \
	include/OAF/CUIManager.h \
	include/OAF/DockResizeHelper.h \
	include/OAF/ViewFindHelper.h \
	include/OAF/CSearchLineWidget.h \
	include/OAF/CWebView.h \
	include/OAF/CProgressWidget.h \
	include/OAF/CElidedLabel.h

#
# Исходные тексты
#
SOURCES += \
	src/CLineEdit.cpp \
	src/CTabWidget.cpp \
	src/CUIManager.cpp \
	src/DockResizeHelper.cpp \
	src/ViewFindHelper.cpp \
	src/CSearchLineWidget.cpp \
	src/CWebView.cpp \
	src/CProgressWidget.cpp \
	src/CUIContainer.cpp \
	src/CElidedLabel.cpp

#
# Формы диалогов
#
FORMS += \
	src/CSearchLineWidget.ui \
	src/CWebView.ui \
	src/CWebSearchWidget.ui

#
# Ресурсы
#
RESOURCES += \
	src/resources/liboaf_gui.qrc
