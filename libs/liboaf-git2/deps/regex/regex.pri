REGEXSRCDIR = $${GIT2DEPSDIR}/regex
INCLUDEPATH+= $${REGEXSRCDIR}

HEADERS += $${REGEXSRCDIR}/config.h \
	$${REGEXSRCDIR}/regex.h \
	$${REGEXSRCDIR}/regex_internal.h

SOURCES += $${REGEXSRCDIR}/regcomp.c \
	$${REGEXSRCDIR}/regex.c \
	$${REGEXSRCDIR}/regex_internal.c \
	$${REGEXSRCDIR}/regexec.c
