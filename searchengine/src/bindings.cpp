# include <pybind11/pybind11.h>
# include "test.h"

namespace py = pybind11;

PYBIND11_MODULE(searchengine, m) {
    py::class_<Test>(m, "Test")
        .def(py::init<int>())
        .def("search", &Test::search, py::call_guard<py::gil_scoped_release>());
}