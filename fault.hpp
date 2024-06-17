#pragma once
#include "parser.hpp"

namespace FLL {

typedef enum _FLL_Node_Value {
  FLL_UNKNOWN = -1,
  FLL_FALSE = 0,
  FLL_TRUE = 1
} FLL_Node_Value;

class Sim {
  std::unordered_map<core::Node*, FLL_Node_Value> _values;
  const core::NodeMap& _node_map;
  
  void run_node(core::Node* node);
  public:
  Sim(const core::NodeMap& node_map) : _node_map(node_map) {
    for (const auto& node : node_map.map) {
      _values[node.second] = FLL_UNKNOWN;
    }
  };
  /**
   * @brief Set the simulation input
   * 
   * @param values vector of input values
   * @throw `std::invalid_argument` if size of `values` does not match the number of inputs
   */
  void set_input(std::vector<FLL_Node_Value>& values) {
    if (values.size() != _node_map.inputs.size()) {
      throw std::invalid_argument("Input size mismatch");
    }
    for (size_t i = 0; i < _node_map.inputs.size(); ++i) {
      _values[_node_map.inputs[i]] = values[i];
    }
  }
  /**
   * @brief Run the simulation
   * 
   * @throw `std::runtime_error` if any of the input is not set
   */
  void run();
};

}
