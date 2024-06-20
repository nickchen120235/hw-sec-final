#include "parser.hpp"

#include <fstream>
#include <map>
#include <string>

namespace Visualization {

std::string get_node_expression(std::unordered_map<const core::Node*, std::string>& dp, const core::Node* node) {

  // if node is already visited, return th expression
  if (dp.find(node) != dp.end()) {
    return dp[node];
  }

  // gate is input, return itself
  if (node->type == core::GateType::INPUT) {
    // std::cout << node->name << ": " << node->name << std::endl;
    dp[node] = std::string(node->name);
    return std::string(node->name);
  }

  // return !(gate)
  if (node->type == core::GateType::NOT) {
    // std::cout << node->inputs[0]->name << ": " << std::string("!(" + node->inputs[0]->name + ")") << std::endl;
    dp[node] = std::string("!(" + get_node_expression(dp, node->inputs[0]) + ")");
    return std::string("!(" + get_node_expression(dp, node->inputs[0]) + ")");
  }

  // return (gate)
  if (node->type == core::GateType::BUF) {
    // std::cout << node->inputs[0]->name << ": " << std::string("!(" + node->inputs[0]->name + ")") << std::endl;
    dp[node] = std::string(get_node_expression(dp, node->inputs[0]));
    return std::string(get_node_expression(dp, node->inputs[0]));
  }

  std::string output = "";
  std::string prefix = "(";

  // if gate is a N__ gate, use prefix "!(" gate)
  if (node->type == core::GateType::NOR || node->type == core::GateType::NAND || node->type == core::GateType::XNOR) {
    prefix = "!(";
  }

  // [input0]
  output = get_node_expression(dp, node->inputs[0]);

  for (std::size_t i = 1; i < node->inputs.size(); ++i) {

    // !([input0]
    output = prefix + output;

    // add operator
    if (node->type == core::GateType::NOR || node->type == core::GateType::OR) {
      output += " | ";
    }
    else if (node->type == core::GateType::NAND || node->type == core::GateType::AND) {
      output += " & ";
    }
    else if (node->type == core::GateType::XOR || node->type == core::GateType::XNOR) {
      output += " ^ ";
    }
    else {
      // node should only be this six gate types.
      std::cout << "Something went wrong" << std::endl;
    }

    // !([input0] [operator] [input1])
    output += get_node_expression(dp, node->inputs[i]) + ")";
  }

  // std::cout << node->name << ": " << output << std::endl;
  dp[node] = output;
  return output;
}

void write_to_verilog_file(const core::NodeMap& node_map, bool show_intermidiate_gate = false) {

  // write to output.v
  std::ofstream file("output.v");

  if (!file.is_open()) {
    exit(1);
  }

  // write module header
  file << "module top(";

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


  // write input gates
  file << "input ";

  for (std::size_t i = 0; i < node_map.inputs.size(); ++i) {
    file << node_map.inputs[i]->name;
    if (i < node_map.inputs.size() - 1) {
      file << ',';
    }
  }

  file << ";\n\n";


  // write output gates
  file << "output ";

  for (std::size_t i = 0; i < node_map.outputs.size(); ++i) {
    file << node_map.outputs[i]->name;
    if (i < node_map.outputs.size() - 1) {
      file << ',';
    }
  }

  file << ";\n\n";

  if (show_intermidiate_gate) {

    // don't reduce intermidiate gates

    // write intermidiate gates
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

    // write output gates
    for (const auto& node : node_map.outputs) {
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
  }
  else {

    std::unordered_map<const core::Node*, std::string> dp_map;

    // write gate expression from output gates
    for (core::Node* node : node_map.outputs) {
      file << "assign " << node->name << " = " << get_node_expression(dp_map, node) << ";\n";
    }

    file << "\nendmodule";
  }

  file.close();
}

} // namespace Visualization