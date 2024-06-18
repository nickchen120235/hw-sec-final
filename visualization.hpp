#include "parser.hpp"

#include <fstream>
#include <string>

namespace Visualization {

void write_to_verilog_file(const core::NodeMap& node_map) {

  std::ofstream file("output.v");

  if (!file.is_open()) {
    exit(1);
  }

  file << "module output_module(";

  for (std::size_t i = 0; i < node_map.inputs.size(); ++i) {
    file << node_map.inputs[i]->name;
    if (i < node_map.inputs.size() - 1) {
      file << ',';
    }
  }

  if (node_map.outputs.size() > 0) {
    file << ',';
  }

  for (std::size_t i = 0; i < node_map.outputs.size(); ++i) {
    file << node_map.outputs[i]->name;
    if (i < node_map.outputs.size() - 1) {
      file << ',';
    }
  }

  file << ");";

  file << "\n\n";

  file << "input ";

  for (std::size_t i = 0; i < node_map.inputs.size(); ++i) {
    file << node_map.inputs[i]->name;
    if (i < node_map.inputs.size() - 1) {
      file << ',';
    }
  }

  file << ";\n\n";

  file << "output ";

  for (std::size_t i = 0; i < node_map.outputs.size(); ++i) {
    file << node_map.outputs[i]->name;
    if (i < node_map.outputs.size() - 1) {
      file << ',';
    }
  }

  file << ";\n\n";

  file << "wire ";

  for (std::size_t i = 0; i < node_map.gates.size(); ++i) {
    file << node_map.gates[i]->name;
    if (i < node_map.gates.size() - 1) {
      file << ',';
    }
  }

  file << ";\n\n";

  for (const auto& node : node_map.gates) {
    switch (node->type) {
#define _(x, y, z, w)                                                                                                  \
  case core::GateType::y:                                                                                              \
    file << w;                                                                                                         \
    break;
      foreach_gate_type_no_in_out
#undef _
    }

    file << "(" << node->name << ",";

    for (std::size_t i = 0; i < node->inputs.size(); ++i) {
      file << node->inputs[i]->name;
      if (i < node->inputs.size() - 1) {
        file << ",";
      }
    }

    file << ");" << std::endl;
  }

  for (const auto& node : node_map.out_gates) {
    switch (node->type) {
#define _(x, y, z, w)                                                                                                  \
  case core::GateType::y:                                                                                              \
    file << w;                                                                                                         \
    break;
      foreach_gate_type_no_in_out
#undef _
    }

    file << "(" << node->name << ",";

    for (std::size_t i = 0; i < node->inputs.size(); ++i) {
      file << node->inputs[i]->name;
      if (i < node->inputs.size() - 1) {
        file << ",";
      }
    }

    file << ");" << std::endl;
  }

  file << "\nendmodule";

  file.close();
}

} // namespace Visualization