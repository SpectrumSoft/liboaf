TARGET   = oaf-boost
TEMPLATE = lib

#
# Build configuration
#
CONFIG += qt thread warn_on
QT -= gui

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
# OAF-ROOT
#
TOPSRCDIR    = ../..
INCLUDEPATH += $${TOPSRCDIR}

#
# Дополнительный путь поиска заголовочных файлов
#
INCLUDEPATH += include

#
# Отключаем поддержку потоков в Boost
#
DEFINES += BOOST_DISABLE_THREADS

#
# Набор флагов для корректной линковкой с ликсом в виндовс
#
win32 {
    #
    # Собираем динамически
    #
    DEFINES += BOOST_ALL_DYN_LINK

    DEFINES  += _CRT_SECURE_NO_WARNINGS NOMINMAX \
		_MBCS WIN32 _WIN32 _WINDOWS \
		HAVE_GETTEXT _CRT_SECURE_NO_DEPRECATE \
		_CRT_NONSTDC_NO_DEPRECATE

    CONFIG(debug,debug|release) {
		DEFINES += _DEBUG
    }

    QMAKE_CXXFLAGS -= -Zc:wchar_t
    QMAKE_CXXFLAGS += /Zc:wchar_t- /GR /Zi /nologo /W4 /WX- /Ob0 /Oy- \
		/Gm- /EHsc /RTC1 /GS /fp:precise /Zc:forScope \
		/errorReport:prompt
    QMAKE_CXXFLAGS += /Gd /TP /wd4288 /wd4355 /wd4800 /wd4996 /wd4267 /wd4180 /wd4100 /wd4231 /analyze- /we4101 /we4189

    QMAKE_LFLAGS   += /INCREMENTAL \
		/SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT \
		/MACHINE:X86 /ERRORREPORT:QUEUE

    QMAKE_CXXFLAGS_DEBUG += /MDd /Fo"support.dir/Debug/"
    QMAKE_CXXFLAGS_RELEASE += /MD /Fo"support.dir/Release/"

    #
    # Убираем флаг оптимизации, т.к. он конфликтует с /RTC1
    #
    QMAKE_CXXFLAGS_RELEASE -= -O2
    QMAKE_LFLAGS_DEBUG += /DEBUG
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON += -W4
    QMAKE_CXXFLAGS += /wd4127 /wd4512 /wd4065
}

#
# Для совместной сборки сигналов Boost и Qt
#
CONFIG += no_keywords

#
# Блокируем предупреждения о неиспользуемых локальных объвлениях типов,
# при сборке старого Boost из состава LyX gcc >= 4.8
#
linux-g++* {
	system( g++ --version | grep -e "\\>[5-9].[0-9].[0-9]" ) {
		# g++ версии 5 или выше
		QMAKE_CXXFLAGS += -Wno-unused-local-typedefs
	} else {
		system( g++ --version | grep -e "[4].[8-9].[0-9]" ) {
			# g++ версии 4.8 или выше
			QMAKE_CXXFLAGS += -Wno-unused-local-typedefs
		} else {
			# g++ версии до 4.7.9
		}
	}
}

#
# Заголовочные файлы
#
HEADERS += \
	include/*

#
# Boost Regex
#
SOURCES += \
	src/regex/cpp_regex_traits.cpp \
	src/regex/c_regex_traits.cpp \
	src/regex/cregex.cpp \
	src/regex/fileiter.cpp \
	src/regex/instances.cpp \
	src/regex/regex.cpp \
	src/regex/regex_raw_buffer.cpp \
	src/regex/regex_traits_defaults.cpp \
	src/regex/w32_regex_traits.cpp \
    src/boost.cpp

#
# Boost Signals
#
SOURCES += \
	src/signals/connection.cpp \
	src/signals/named_slot_map.cpp \
	src/signals/signal_base.cpp \
	src/signals/slot.cpp \
	src/signals/trackable.cpp
