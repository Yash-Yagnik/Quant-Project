#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "lumina/avellaneda_stoikov.hpp"
#include "lumina/order_book_imbalance.hpp"
#include "lumina/simd_indicators.hpp"
#include "lumina/types.hpp"

namespace py = pybind11;

PYBIND11_MODULE(lumina_py, m) {
  m.doc() = "Lumina-HFT Python bindings";

  py::class_<lumina::AvellanedaStoikov>(m, "AvellanedaStoikov")
    .def(py::init<double, double, double>(),
         py::arg("gamma"), py::arg("sigma"), py::arg("T_seconds"))
    .def("reservation_price", &lumina::AvellanedaStoikov::reservation_price,
         py::arg("s"), py::arg("t"), py::arg("q"))
    .def("optimal_half_spread", &lumina::AvellanedaStoikov::optimal_half_spread,
         py::arg("k"))
    .def("get_quotes",
         [](lumina::AvellanedaStoikov& as, double s, double t, double q, double k, double obi) {
           double bid_off, ask_off;
           as.get_quotes(s, t, q, k, obi, bid_off, ask_off);
           return py::make_tuple(bid_off, ask_off);
         },
         py::arg("s"), py::arg("t"), py::arg("q"), py::arg("k"), py::arg("obi_skew") = 0.0)
    .def_property("sigma", nullptr, &lumina::AvellanedaStoikov::set_sigma)
    .def_property("gamma", nullptr, &lumina::AvellanedaStoikov::set_gamma)
    .def_property("T", nullptr, &lumina::AvellanedaStoikov::set_T);

  m.def("order_book_imbalance", &lumina::order_book_imbalance,
        py::arg("bid_volume"), py::arg("ask_volume"));

  py::class_<lumina::OBISignal>(m, "OBISignal")
    .def(py::init<double>(), py::arg("alpha") = 0.1)
    .def("update", &lumina::OBISignal::update,
         py::arg("bid_volume"), py::arg("ask_volume"))
    .def("value", &lumina::OBISignal::value)
    .def("reset", &lumina::OBISignal::reset);

  m.def("variance_simd",
        [](py::array_t<double> arr) {
          auto buf = arr.unchecked<1>();
          return lumina::variance_simd(buf.data(0), buf.shape(0));
        });
  m.def("ema_simd",
        [](py::array_t<double> data, double alpha) {
          auto in = data.unchecked<1>();
          py::array_t<double> out(data.size());
          auto out_buf = out.mutable_unchecked<1>();
          lumina::ema_simd(in.data(0), out_buf.mutable_data(0), in.shape(0), alpha);
          return out;
        },
        py::arg("data"), py::arg("alpha"));
  m.def("sum_simd",
        [](py::array_t<double> arr) {
          auto buf = arr.unchecked<1>();
          return lumina::sum_simd(buf.data(0), buf.shape(0));
        });
}
