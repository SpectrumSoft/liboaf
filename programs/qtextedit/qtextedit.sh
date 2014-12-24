#!/bin/sh

TOPDIR=../..
OAFROOT=$TOPDIR

#
# Библиотеки и плагины
#
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-core/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-boost/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-git2/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-oql/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-std/debug
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OAFROOT/libs/liboaf-gui/debug
export OAFPATH="$OAFPATH;$OAFROOT/plugins/liboaf-io/debug"
export OAFPATH="$OAFPATH;$OAFROOT/plugins/liboaf-std/debug"
export OAFPATH="$OAFPATH;$OAFROOT/plugins/liboaf-txt/debug"

#
# Устанавливаем пути к базе MIME-типов и значкам для них
#
export OAFMIMES="$OAFROOT/share/mime/packages"
export OAFICONS="$OAFROOT/share/icons"

#
# Переводы
#
export OAF_TR_DIR=$OAFROOT/translate

./debug/qtextedit "$@"
