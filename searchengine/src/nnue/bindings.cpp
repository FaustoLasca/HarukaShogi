#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "load.h"


namespace py = pybind11;


namespace harukashogi {
namespace NNUE {


template<typename T>
py::array_t<T> make_1d_view(const std::vector<T>& vec, py::object owner) {
    return py::array_t<T>(
        { static_cast<py::ssize_t>(vec.size()) },
        { static_cast<py::ssize_t>(sizeof(T)) },
        vec.data(),
        owner
    );
}


template<typename T>
py::array_t<T> make_2d_view(const std::vector<T>& vec, size_t rows, size_t cols, py::object owner) {
    return py::array_t<T>(
        { static_cast<py::ssize_t>(rows), static_cast<py::ssize_t>(cols) },
        { static_cast<py::ssize_t>(sizeof(T) * cols), static_cast<py::ssize_t>(sizeof(T)) },
        vec.data(),
        owner
    );
}


PYBIND11_MODULE(nnue_loader, m) {
    py::class_<DataBatch, std::shared_ptr<DataBatch>>(m, "DataBatch")
        .def_readonly("batch_size", &DataBatch::batch_size)
        .def_property_readonly("black_indexes", [](py::object self) {
            auto& b = self.cast<DataBatch&>();
            return make_2d_view(b.black_indexes, b.batch_size, ACTIVE_FEATURES, self);
        })
        .def_property_readonly("white_indexes", [](py::object self) {
            auto& b = self.cast<DataBatch&>();
            return make_2d_view(b.white_indexes, b.batch_size, ACTIVE_FEATURES, self);
        })
        .def_property_readonly("scores", [](py::object self) {
            auto& b = self.cast<DataBatch&>();
            return make_1d_view(b.scores, self);
        })
        .def_property_readonly("results", [](py::object self) {
            auto& b = self.cast<DataBatch&>();
            return make_1d_view(b.results, self);
        })
        .def_property_readonly("stms", [](py::object self) {
            auto& b = self.cast<DataBatch&>();
            return make_1d_view(b.stms, self);
        });


    m.def("load_data_batch",
        &load_data_batch,
        py::arg("file_path"),
        py::call_guard<py::gil_scoped_release>()
    );
}


} // namespace NNUE
} // namespace harukashogi