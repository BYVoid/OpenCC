use ExtUtils::MakeMaker;

WriteMakefile(
    'NAME'   => 'marisa',
    'LIBS'   => ['-lmarisa'],
    'OBJECT' => 'marisa-swig.o marisa-swig_wrap.o'
);
