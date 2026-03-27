%module marisa

%include "cstring.i"
%include "exception.i"

%{
#include "marisa-swig.h"
%}

%apply (char *STRING, int LENGTH) { (const char *ptr, std::size_t length) };

%cstring_output_allocate_size(const char **ptr_out, std::size_t *length_out, );
%cstring_output_allocate_size(const char **ptr_out_to_be_deleted,
    std::size_t *length_out, delete [] (*$1));

%exception {
  try {
    $action
  } catch (const marisa::Exception &ex) {
    SWIG_exception(SWIG_RuntimeError, ex.what());
  } catch (...) {
    SWIG_exception(SWIG_UnknownError,"Unknown exception");
  }
}

%include "marisa-swig.h"

%constant size_t INVALID_KEY_ID = MARISA_INVALID_KEY_ID;
