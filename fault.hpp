#pragma once
#include "parser.hpp"
#include <tuple>

// Fault Analysis-Based Logic Locking
namespace FLL {

typedef enum _FLL_Node_Value {
  FLL_UNKNOWN = -1,
  FLL_FALSE = 0,
  FLL_TRUE = 1
} FLL_Node_Value;

typedef std::vector<FLL_Node_Value> SimulationValues;
class Sim {
  std::unordered_map<core::Node*, FLL_Node_Value> _values;
  const core::NodeMap& _node_map;
  core::Node* _fault_node = nullptr;
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
  void set_input(SimulationValues& values) {
    if (values.size() != _node_map.inputs.size()) {
      throw std::invalid_argument("Input size mismatch");
    }
    for (std::size_t i = 0; i < _node_map.inputs.size(); ++i) {
      // std::cout << "Setting input " << _node_map.inputs[i]->name << " to " << values[i] << std::endl;
      _values[_node_map.inputs[i]] = values[i];
    }
  }
  /**
   * @brief Set the fault node
   * 
   * @param fault_node pointer to the node with stuck-at fault
   */
  void set_fault(core::Node* fault_node, FLL_Node_Value value) {
    _fault_node = fault_node;
    _values[_fault_node] = value;
  }
  /**
   * @brief Get the fault node
   * 
   * @return `core::Node*` pointer to the node with stuck-at fault
   */
  core::Node* get_fault() {
    return _fault_node;
  }
  /**
   * @brief Get simulation values
   * 
   * @return `std::unordered_map<core::Node*, FLL_Node_Value>&` simulation values
   */
  std::unordered_map<core::Node*, FLL_Node_Value>& get_values() {
    return _values;
  }
  /**
   * @brief Run the simulation
   * 
   * @throw `std::runtime_error` if any of the input is not set
   */
  void run();
};

// (NoP0, NoO0, NoP1, NoO1)
typedef std::tuple<unsigned long, unsigned long, unsigned long, unsigned long> FaultImpactValueTuple;
typedef std::pair<core::Node*, unsigned long> FaultImpactResultValuePair;
class FaultImpactAnalysis {
  std::unordered_map<core::Node*, FaultImpactValueTuple> _fault_impact;
  std::vector<FaultImpactResultValuePair> _res;
  const core::NodeMap& _node_map;

  public:
  FaultImpactAnalysis(const core::NodeMap& node_map) : _node_map(node_map) {
    for (const auto& node : node_map.map) {
      _fault_impact[node.second] = std::make_tuple(0, 0, 0, 0);
    }
  };
  void run(u_int32_t rounds, u_int64_t seed);
  void show() {
    for (const auto& entry: _fault_impact) {
      std::cout << entry.first->name << ": " << std::get<0>(entry.second) << ", " << std::get<1>(entry.second) << ", " << std::get<2>(entry.second) << ", " << std::get<3>(entry.second) << std::endl;
    }
  }
  std::vector<FaultImpactResultValuePair>& get_res() {
    return _res;
  }
};

/**
 * @brief Lock the circuit with `keyBits` bits
 * 
 * @param map Loaded circuit
 * @param keyBits Number of bits of the key
 */
void lock_n_gates(core::NodeMap& map, std::size_t keyBits, u_int32_t rounds, u_int64_t seed);

/**
 * @brief Lock the circuit by percentage
 * 
 * @param map Loaded circuit
 * @param percentage Percentage of lockable nodes
 */
void lock_by_percentage(core::NodeMap& map, float percentage, u_int32_t rounds, u_int64_t seed);

}
