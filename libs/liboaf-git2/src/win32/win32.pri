WIN32SRCDIR = $${GIT2SRCDIR}/win32
INCLUDEPATH  += $${WIN32SRCDIR}

HEADERS += $${WIN32SRCDIR}/dir.h \
		$${WIN32SRCDIR}/error.h \
		$${WIN32SRCDIR}/findfile.h \
		$${WIN32SRCDIR}/mingw-compat.h \
		$${WIN32SRCDIR}/msvc-compat.h \
		$${WIN32SRCDIR}/posix.h \
		$${WIN32SRCDIR}/precompiled.h \
		$${WIN32SRCDIR}/pthread.h \
		$${WIN32SRCDIR}/utf-conv.h \
		$${WIN32SRCDIR}/version.h


SOURCES += $${WIN32SRCDIR}/dir.c \
		$${WIN32SRCDIR}/error.c \
		$${WIN32SRCDIR}/findfile.c \
		$${WIN32SRCDIR}/map.c \
		$${WIN32SRCDIR}/posix_w32.c \
		$${WIN32SRCDIR}/precompiled.c \
		$${WIN32SRCDIR}/pthread.c \
		$${WIN32SRCDIR}/utf-conv.c \


RC_FILE = $${WIN32SRCDIR}/git2.rc
