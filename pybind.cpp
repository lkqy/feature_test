#include <memory>
#include <string>
#include "pybind11/pybind11.h"
#include "src/ads_feature.h"

namespace py = pybind11;

class PyFeatureExtract {
public:
    PyFeatureExtract(const std::string& configs) {}
    std::string ParseAsJsonString(const std::string& input) {
        auto mf = std::make_shared<ModelFeature>();
        return mf->extract_json(input);
    }
	py::bytes ParseTFExample(const std::string& input) {
        auto mf = std::make_shared<ModelFeature>();
        auto s = mf->extract_tf_example(input);
		return py::bytes(s);
    }
private:
};

PYBIND11_MODULE(libpyfeature_extract, m) {
    m.doc() = "feature extract for python";
    py::class_<PyFeatureExtract>(m, "PyFeatureExtract")
        .def(py::init<const std::string &>())
        .def("extract", &PyFeatureExtract::ParseAsJsonString)
        .def("extract_tf_example", &PyFeatureExtract::ParseTFExample);
};

