#!/bin/bash
#Script corrects all calls to giterr_set(xxx,yyy) to giterr_set(xxx,QObject::tr(yyy))

pushd src

find . -type f -name '*.c' | while read i; do
cat $i | sed -f ../libgit_fix.sed > $i.fix
mv -f $i.fix $i
done

popd