#include <pybind11/pybind11.h>
#include "opencc.h"

namespace py = pybind11;

PYBIND11_MODULE(opencc_clib, m) {
    py::class_<opencc::SimpleConverter>(m, "_OpenCC")
        .def(py::init<const std::string&>())
        .def(
            "convert",
            py::overload_cast<const char*, size_t>
            (&opencc::SimpleConverter::Convert, py::const_)
        );

#ifdef VERSION
    m.attr("__version__") = VERSION;
#else
    m.attr("__version__") = "dev";
#endif
}
