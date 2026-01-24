# include <pybind11/pybind11.h>

# include "test.h"
# include "perft.h"

namespace py = pybind11;

namespace harukashogi {

PYBIND11_MODULE(searchengine, m) {
    py::class_<Test>(m, "Test")
        .def(py::init<int>())
        .def("search", &Test::search, py::call_guard<py::gil_scoped_release>());

    m.def("perft", [](const std::string& sfen, int depth) {
        return perft(sfen, depth);
    }, py::arg("sfen"), py::arg("depth"));
}

} // namespace harukashogi