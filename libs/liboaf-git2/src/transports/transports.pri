TRANSPORTSDIR = $${GIT2SRCDIR}/transports
INCLUDEPATH+= $${TRANSPORTSDIR}

HEADERS += $${TRANSPORTSDIR}/smart.h


SOURCES += $${TRANSPORTSDIR}/cred.c \
	$${TRANSPORTSDIR}/cred_helpers.c \
	$${TRANSPORTSDIR}/git.c \
	$${TRANSPORTSDIR}/http.c \
	$${TRANSPORTSDIR}/local.c \
	$${TRANSPORTSDIR}/smart.c \
	$${TRANSPORTSDIR}/smart_pkt.c \
	$${TRANSPORTSDIR}/smart_protocol.c \
	$${TRANSPORTSDIR}/ssh.c \
	$${TRANSPORTSDIR}/winhttp.c
