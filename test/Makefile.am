link = ln -sf
opencc = $(top_builddir)/src/opencc
dictionary = $(top_builddir)/src/*.ocd
config = $(top_srcdir)/data/config/*.ini
check_sh = $(top_srcdir)/test/check.sh

testcases = \
	zhs2zht.in \
	zhs2zht.ans \
	zht2zhs.in \
	zht2zhs.ans \
	$(NULL)

EXTRA_DIST = \
	check.sh \
	$(testcases) \
	$(NULL)

all: all-am test

test: $(opencc) $(dictionary) $(config) $(check_sh) $(testcases)
	$(link) $(opencc) .
	$(link) $(dictionary) .
	$(link) $(config) .
	$(SHELL) $(check_sh) zhs2zht
	$(SHELL) $(check_sh) zht2zhs
