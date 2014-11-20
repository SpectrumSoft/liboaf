XDIFFDIR = $${GIT2SRCDIR}/xdiff
INCLUDEPATH+= $${XDIFFDIR}

HEADERS += $${XDIFFDIR}/xdiff.h \
		$${XDIFFDIR}/xdiffi.h \
		$${XDIFFDIR}/xemit.h \
		$${XDIFFDIR}/xinclude.h \
		$${XDIFFDIR}/xmacros.h \
		$${XDIFFDIR}/xprepare.h \
		$${XDIFFDIR}/xtypes.h \
		$${XDIFFDIR}/xutils.h


SOURCES += $${XDIFFDIR}/xdiffi.c \
		$${XDIFFDIR}/xemit.c \
		$${XDIFFDIR}/xhistogram.c \
		$${XDIFFDIR}/xmerge.c \
		$${XDIFFDIR}/xpatience.c \
		$${XDIFFDIR}/xprepare.c \
		$${XDIFFDIR}/xutils.c \
