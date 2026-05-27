from setuptools import setup, Extension

marisa_module = Extension("_marisa",
                          sources=["marisa-swig-python3_wrap.cxx",
                                   "marisa-swig-python3.cxx"],
                          libraries=["marisa"])

setup(name = "marisa",
      ext_modules = [marisa_module],
      py_modules = ["marisa"])
