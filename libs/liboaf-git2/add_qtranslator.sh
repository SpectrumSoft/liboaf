#!/bin/bash
#
# Скрипт для преобразования всех вызовов giterr_set(xxx,yyy) в вызов
# giterr_set(xxx,Q_TRANSLATE_NOOP("libgit2", yyy))
#

pushd src

find . -type f -name '*.c' | while read i; do
	cat $i | sed -f ../libgit_fix.sed > $i.fix
	mv -f $i.fix $i
done

popd