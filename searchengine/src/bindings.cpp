# include <pybind11/pybind11.h>

# include "test.h"
# include "perft.h"

namespace py = pybind11;

namespace harukashogi {

PYBIND11_MODULE(searchengine, m) {
    py::class_<Test>(m, "Test")
        .def(py::init<int>())
        .def("search", &Test::search, py::call_guard<py::gil_scoped_release>());

    m.def("perft", py::overload_cast<std::string, int>(&perft));
}

} // namespace harukashogi