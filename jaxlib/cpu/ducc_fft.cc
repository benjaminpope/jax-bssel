/* Copyright 2020 The JAX Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <complex>
#include <vector>

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"
#include "jaxlib/cpu/ducc_fft_generated.h"
#include "jaxlib/cpu/ducc_fft_kernels.h"
#include "jaxlib/kernel_pybind11_helpers.h"

namespace py = pybind11;

namespace jax {
namespace {


py::bytes BuildDynamicDuccFftDescriptor(
    const uint32_t ndims,
    bool is_double, int fft_type,
    const std::vector<uint32_t> &axes,
    bool forward) {
  DynamicDuccFftDescriptorT descriptor;
  descriptor.ndims = ndims;
  descriptor.fft_type = static_cast<DuccFftType>(fft_type);
  descriptor.dtype =
      is_double ? DuccFftDtype_COMPLEX128 : DuccFftDtype_COMPLEX64;
  descriptor.axes = axes;
  descriptor.forward = forward;
  flatbuffers::FlatBufferBuilder fbb;
  fbb.Finish(DynamicDuccFftDescriptor::Pack(fbb, &descriptor));
  return py::bytes(reinterpret_cast<char *>(fbb.GetBufferPointer()),
                   fbb.GetSize());
}

py::dict Registrations() {
  pybind11::dict dict;
  // TODO(b/287702203): this must be kept until EOY 2023 for backwards
  // of serialized functions using fft.
  dict["ducc_fft"] = EncapsulateFunction(DuccFft);
  dict["dynamic_ducc_fft"] = EncapsulateFunction(DynamicDuccFft);
  return dict;
}

PYBIND11_MODULE(_ducc_fft, m) {
  m.def("registrations", &Registrations);
  m.def("dynamic_ducc_fft_descriptor", &BuildDynamicDuccFftDescriptor,
        py::arg("ndims"), py::arg("is_double"), py::arg("fft_type"),
        py::arg("axes"), py::arg("forward"));
}

}  // namespace
}  // namespace jax
