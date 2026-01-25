# include <pybind11/pybind11.h>

# include "test.h"
# include "perft.h"
# include "search.h"

namespace py = pybind11;

namespace harukashogi {

PYBIND11_MODULE(searchengine, m) {
    py::class_<Test>(m, "Test")
        .def(py::init<int>())
        .def("search", &Test::search, py::call_guard<py::gil_scoped_release>());

    py::class_<Searcher>(m, "Searcher")
        .def(py::init<>())
        .def("set_position", &Searcher::set_position)
        .def("search", py::overload_cast<int, int>(&Searcher::search),
            py::arg("time_limit"),
            py::arg("max_depth"),
            py::call_guard<py::gil_scoped_release>());

    m.def("perft", py::overload_cast<std::string, int>(&perft));
}

} // namespace harukashogi