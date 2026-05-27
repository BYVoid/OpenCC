from distutils.core import setup, Extension

marisa_module = Extension("_marisa",
                          sources=["marisa-swig_wrap.cxx", "marisa-swig.cxx"],
                          libraries=["marisa"])

setup(name = "marisa",
      ext_modules = [marisa_module],
      py_modules = ["marisa"])
