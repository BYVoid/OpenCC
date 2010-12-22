#!/bin/sh

xgettext \
	--default-domain="opencc" \
	--directory=".." \
	--force-po \
	--add-comments="TRANSLATORS:" \
	--keyword=_ --keyword=N_ \
	--files-from="POTFILES.in" \
	--copyright-holder="BYVoid <byvoid.kcp@gmail.com>" \
	--msgid-bugs-address="http://code.google.com/p/open-chinese-convert/issues/entry" \
	--from-code=UTF-8 \
	--sort-by-file \
	--output=opencc.pot 

for LANG in `cat LINGUAS`
do
	echo -n $LANG
	msgmerge \
		--backup=none \
		--update $LANG.po \
		opencc.pot
done

rm opencc.pot