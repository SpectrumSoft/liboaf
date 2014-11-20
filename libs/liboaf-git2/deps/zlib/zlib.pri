ZLIBDIR = $${GIT2DEPSDIR}/zlib
INCLUDEPATH += $${ZLIBDIR}

HEADERS += $${ZLIBDIR}/crc32.h \
	$${ZLIBDIR}/deflate.h \
	$${ZLIBDIR}/inffast.h \
	$${ZLIBDIR}/inffixed.h \
	$${ZLIBDIR}/inflate.h \
	$${ZLIBDIR}/inftrees.h \
	$${ZLIBDIR}/trees.h \
	$${ZLIBDIR}/zconf.h \
	$${ZLIBDIR}/zlib.h \
	$${ZLIBDIR}/zutil.h


SOURCES += $${ZLIBDIR}/adler32.c \
	$${ZLIBDIR}/crc32.c \
	$${ZLIBDIR}/deflate.c \
	$${ZLIBDIR}/inffast.c \
	$${ZLIBDIR}/inflate.c \
	$${ZLIBDIR}/inftrees.c \
	$${ZLIBDIR}/trees.c \
	$${ZLIBDIR}/zutil.c
