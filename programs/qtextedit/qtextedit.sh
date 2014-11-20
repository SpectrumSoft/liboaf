#!/bin/sh

TOPDIR=../..

OAFROOT=$TOPDIR
#
# Устанавливаем пути к базе MIME-типов и значкам для них
#
export OAFMIMES="$OAFROOT/share/mime/packages"
export OAFICONS="$OAFROOT/share/icons"

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-core/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-oql/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-std/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-git2/debug
export OAFPATH="$OAFPATH;$OAFROOT/plugins/liboaf-io/debug"
export OAFPATH="$OAFPATH;$OAFROOT/plugins/liboaf-std/debug"
export OAFPATH="$OAFPATH;$OAFROOT/plugins/liboaf-txt/debug"

export LYX_DIR_20x=$TOPDIR/../qpiket.git/plugins/libpiket-lyx.git/src-lyx/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/../qpiket.git/libs/libpiket-utils/debug
export OAFPATH="$OAFPATH;$OAFROOT/../qpiket.git/plugins/libpiket-lyx.git/debug"

./debug/qtextedit "$@"
