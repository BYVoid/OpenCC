#include <pybind11/pybind11.h>
#include "opencc.h"

class PYOpenCC {
private:
    opencc_t converter;

public:
    std::string config;

    PYOpenCC(const char* configFileName) {
        config = configFileName;
        converter = opencc_open(configFileName);
        if (converter == (opencc_t) - 1) {
            throw std::invalid_argument(opencc_error());
        };
    }

    ~PYOpenCC() {
        opencc_close(converter);
    }

    char* convert(const char* text, size_t length) {
        return opencc_convert_utf8(converter, text, length);
    };
};

namespace py = pybind11;

PYBIND11_MODULE(opencc_clib, m) {
    m.doc() = R"pbdoc(
        OpenCC Pybind11 plugin
        -----------------------
        .. currentmodule:: opencc_clib
        .. autosummary::
           :toctree: _generate
           PYOpenCC
    )pbdoc";

    py::class_<PYOpenCC>(m, "_OpenCC")
        .def(py::init<const char*>())
        .def_readonly("config", &PYOpenCC::config)
        .def("convert", &PYOpenCC::convert);

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
