#include "parser.hpp"

#include <fstream>
#include <string>

namespace Visualization {

std::string get_node_expression(const core::Node* node) {

  if (node->type == core::GateType::INPUT) {
    // std::cout << node->name << ": " << node->name << std::endl;
    return std::string(node->name);
  }

  if (node->type == core::GateType::NOT) {
    // std::cout << node->inputs[0]->name << ": " << std::string("!(" + node->inputs[0]->name + ")") << std::endl;
    return std::string("!(" + get_node_expression(node->inputs[0]) + ")");
  }

  if (node->type == core::GateType::BUF) {
    std::cout << node->inputs[0]->name << ": " << std::string("!(" + node->inputs[0]->name + ")") << std::endl;
    return std::string(get_node_expression(node->inputs[0]));
  }

  std::string output = "";
  std::string prefix = "(";

  if (node->type == core::GateType::NOR || node->type == core::GateType::NAND || node->type == core::GateType::XNOR) {
    prefix = "!(";
  }

  output = get_node_expression(node->inputs[0]);

  for (std::size_t i = 1; i < node->inputs.size(); ++i) {

    output = prefix + output;

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
      std::cout << "Someting went wrong" << std::endl;
    }

    output += get_node_expression(node->inputs[i]) + ")";
  }

  // std::cout << node->name << ": " << output << std::endl;
  return output;
}

void write_to_verilog_file(const core::NodeMap& node_map) {

  std::ofstream file("output.v");

  if (!file.is_open()) {
    exit(1);
  }

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

  for (core::Node* node : node_map.outputs) {
    file << "assign " << node->name << " = " << get_node_expression(node) << ";\n";
  }

  file << "\nendmodule";

  file.close();
}

} // namespace Visualization