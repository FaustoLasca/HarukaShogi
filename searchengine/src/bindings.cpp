# include <pybind11/pybind11.h>

# include "perft.h"
# include "search.h"
# include "misc.h"

namespace py = pybind11;

namespace harukashogi {

PYBIND11_MODULE(haruka, m) {
    // py::class_<Searcher>(m, "Searcher", py::module_local())
    //     .def(py::init<bool>(), py::arg("useOpeningBook")=false)
    //     .def("set_position", &Searcher::set_position)
    //     .def("search", py::overload_cast<int, int>(&Searcher::search),
    //         py::arg("time_limit"),
    //         py::arg("max_depth"),
    //         py::call_guard<py::gil_scoped_release>())
    //     .def("print_stats", &Searcher::print_stats),

    m.def("perft", py::overload_cast<std::string, int>(&perft));

    m.def("init", &init);
}

} // namespace harukashogi