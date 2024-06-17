#include "fault.hpp"
#include <stack>
#include <algorithm>
#include <numeric>

using core::GateType;

namespace FLL {

void Sim::run_node(core::Node* node) {
  // std::cout << "Running node " << node->name << std::endl;
  if (node->inputs.size() == 0) return;
  if (std::any_of(node->inputs.begin(), node->inputs.end(), [node, this](core::Node* n) { return this->_values[n] == FLL_UNKNOWN; })) {
    this->_values[node] = FLL_UNKNOWN;
    return;
  }
  switch (node->type) {
    case GateType::NOT:
      this->_values[node] = this->_values[node->inputs[0]] == FLL_TRUE ? FLL_FALSE : FLL_TRUE;
      break;
    case GateType::BUF:
      this->_values[node] = this->_values[node->inputs[0]];
      break;
    case GateType::AND:
      this->_values[node] = std::all_of(node->inputs.begin(), node->inputs.end(), [this](core::Node* n) { return this->_values[n]; }) ? FLL_TRUE : FLL_FALSE;
      break;
    case GateType::NAND:
      this->_values[node] = std::all_of(node->inputs.begin(), node->inputs.end(), [this](core::Node* n) { return this->_values[n]; }) ? FLL_FALSE : FLL_TRUE;
      break;
    case GateType::OR:
      this->_values[node] = std::any_of(node->inputs.begin(), node->inputs.end(), [this](core::Node* n) { return this->_values[n]; }) ? FLL_TRUE : FLL_FALSE;
      break;
    case GateType::NOR:
      this->_values[node] = std::any_of(node->inputs.begin(), node->inputs.end(), [this](core::Node* n) { return this->_values[n]; }) ? FLL_FALSE : FLL_TRUE;
      break;
    case GateType::XOR:
      this->_values[node] = std::accumulate(
        node->inputs.begin(), node->inputs.end(), FLL_FALSE,
        [this](FLL_Node_Value cur, core::Node* next) { return cur == this->_values[next] ? FLL_FALSE : FLL_TRUE; });
      break;
    case GateType::XNOR:
      this->_values[node] = std::accumulate(
        node->inputs.begin(), node->inputs.end(), FLL_FALSE,
        [this](FLL_Node_Value cur, core::Node* next) { return cur == this->_values[next] ? FLL_TRUE : FLL_FALSE; });
      break;
    default:
      break;
  }
  // std::cout << node->name << " = " << this->_values[node] << std::endl;
}

void Sim::run() {
  // do sanity check before starting simulation
  if (std::any_of(this->_node_map.inputs.begin(), this->_node_map.inputs.end(), [this](core::Node* n) { return this->_values[n] == FLL_UNKNOWN; })) {
    throw std::runtime_error("Circuit has unknown inputs");
  }

  // initialize DFS
  std::stack<core::Node*> s;
  for (const auto& output: this->_node_map.outputs)
    s.push(output);

  while (!s.empty()) {
    core::Node* node = s.top();
    // std::cout << node->name << std::endl;
    s.pop();
    if (std::all_of(node->inputs.begin(), node->inputs.end(), [this](core::Node* n) { return this->_values[n] != FLL_UNKNOWN; })) {
      this->run_node(node);
    }
    else {
      s.push(node);
      for (const auto& input: node->inputs) {
        if (this->_values[input] == FLL_UNKNOWN) {
          s.push(input);
        }
      }
    }
  }
}

}
