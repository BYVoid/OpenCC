link = ln -sf
opencc = $(top_builddir)/src/opencc
dictionary = $(top_builddir)/src/*.ocd
config = $(top_srcdir)/data/config/*.ini
check_sh = $(top_srcdir)/test/check.sh

testcases_dir = $(top_srcdir)/test/testcases
zhs2zht_in = $(testcases_dir)/zhs2zht.in
zhs2zht_ans = $(testcases_dir)/zhs2zht.ans
zht2zhs_in = $(testcases_dir)/zht2zhs.in
zht2zhs_ans = $(testcases_dir)/zht2zhs.ans
mix2zht_in = $(testcases_dir)/mix2zht.in
mix2zht_ans = $(testcases_dir)/mix2zht.ans
mix2zhs_in = $(testcases_dir)/mix2zhs.in
mix2zhs_ans = $(testcases_dir)/mix2zhs.ans

testcases = \
	$(zhs2zht_in) \
	$(zhs2zht_ans) \
	$(zht2zhs_in) \
	$(zht2zhs_ans) \
	$(mix2zht_in) \
	$(mix2zht_ans) \
	$(mix2zhs_in) \
	$(mix2zhs_ans) \
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
	$(link) $(zhs2zht_in) .
	$(link) $(zhs2zht_ans) .
	$(link) $(zht2zhs_in) .
	$(link) $(zht2zhs_ans) .
	$(link) $(mix2zht_in) .
	$(link) $(mix2zht_ans) .
	$(link) $(mix2zhs_in) .
	$(link) $(mix2zhs_ans) .
	$(SHELL) $(check_sh) zhs2zht
	$(SHELL) $(check_sh) zht2zhs
	$(SHELL) $(check_sh) mix2zht
	$(SHELL) $(check_sh) mix2zhs
