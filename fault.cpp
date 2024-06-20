#include "fault.hpp"
#include <stack>
#include <algorithm>
#include <numeric>
#include <cmath>

using core::GateType;

namespace FLL {

void Sim::run_node(core::Node* node) {
  // std::cout << "Running node " << node->name << std::endl;
  if (node->inputs.size() == 0) return;
  if (node == this->_fault_node) return;
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

void FaultImpactAnalysis::run(u_int64_t seed = time(0)) {
  std::cout << "Running fault impact analysis" << std::endl;
  std::srand(seed);
  for (unsigned long i = 0; i < 1000; ++i) {
    std::cout << "\rIteration: " << i + 1 << " / 1000";
    std::cout.flush();
    // prepare input
    SimulationValues inputs(this->_node_map.inputs.size());
    for (unsigned long j = 0; j < this->_node_map.inputs.size(); ++j) {
      inputs[j] = std::rand() % 2 == 1 ? FLL_TRUE : FLL_FALSE;
    }

    // run simulation without fault
    Sim orig(this->_node_map);
    orig.set_input(inputs);
    orig.run();
    SimulationValues orig_outputs(this->_node_map.outputs.size());
    for (const auto& output: this->_node_map.outputs) {
      orig_outputs.push_back(orig.get_values()[output]);
    }
    // for (const auto& output: orig_outputs) {
    //   std::cout << output << " ";
    // }
    // std::cout << std::endl;
    for (const auto& map_entry: this->_node_map.map) {
      unsigned long nop0, noo0, nop1, noo1;
      std::tie(nop0, noo0, nop1, noo1) = this->_fault_impact[map_entry.second];
      // run simulation with stuck at 0
      Sim fault0(this->_node_map);
      fault0.set_input(inputs);
      fault0.set_fault(map_entry.second, FLL_FALSE);
      fault0.run();
      SimulationValues fault0_outputs(this->_node_map.outputs.size());
      for (const auto& output: this->_node_map.outputs) {
        fault0_outputs.push_back(fault0.get_values()[output]);
      }
      // for (const auto& output: fault0_outputs) {
      //   std::cout << output << " ";
      // }
      // std::cout << std::endl;
      // run simulation with stuck at 1
      Sim fault1(this->_node_map);
      fault1.set_input(inputs);
      fault1.set_fault(map_entry.second, FLL_TRUE);
      fault1.run();
      SimulationValues fault1_outputs(this->_node_map.outputs.size());
      for (const auto& output: this->_node_map.outputs) {
        fault1_outputs.push_back(fault1.get_values()[output]);
      }
      // for (const auto& output: fault1_outputs) {
      //   std::cout << output << " ";
      // }
      // std::cout << std::endl;
      // calculate fault impact
      SimulationValues diff0(this->_node_map.outputs.size());
      SimulationValues diff1(this->_node_map.outputs.size());
      auto it0 = std::set_difference(orig_outputs.begin(), orig_outputs.end(), fault0_outputs.begin(), fault0_outputs.end(), diff0.begin());
      auto it1 = std::set_difference(orig_outputs.begin(), orig_outputs.end(), fault1_outputs.begin(), fault1_outputs.end(), diff1.begin());
      diff0.resize(it0 - diff0.begin());
      diff1.resize(it1 - diff1.begin());
      // std::cout << "diff0: " << diff0.size() << " diff1: " << diff1.size() << std::endl;
      if (diff0.size() > 0) {
        nop0 += 1; noo0 += diff0.size();
      }
      if (diff1.size() > 0) {
        nop1 += 1; noo1 += diff1.size();
      }
      this->_fault_impact[map_entry.second] = std::make_tuple(nop0, noo0, nop1, noo1);
    }
  }
  std::cout << std::endl;
  std::cout << "Calulating fault impact" << std::endl;
  for (const auto& entry: this->_fault_impact) {
    unsigned long nop0, noo0, nop1, noo1;
    std::tie(nop0, noo0, nop1, noo1) = entry.second;
    this->_res.push_back(std::make_pair(entry.first, nop0 * noo0 + nop1 * noo1));
  }
  std::cout << "Sorting results" << std::endl;
  std::sort(this->_res.begin(), this->_res.end(), [](const FaultImpactResultValuePair& a, const FaultImpactResultValuePair& b) {
    return a.second > b.second;
  });
  std::cout << "Done." << std::endl;
}

void lock_n_gates(core::NodeMap& map, std::size_t keyBits, u_int64_t seed = time(0)) {
  std::cout << "Locking using Fault Analysis-Based Logic Locking" << std::endl;
  // prepare key
  std::srand(seed);
  std::size_t nBits = std::min(keyBits, map.map.size());
  if (nBits != keyBits) {
    std::cerr << "Warning keyBits is larger than the number of lockable nodes." << std::endl;
  }
  std::vector<bool> key(nBits);
  std::generate(key.begin(), key.end(), []() { return std::rand() % 2; });
  std::cout << "Key: ";
  for (const auto& bit : key) {
    std::cout << (bit ? "1" : "0");
  }
  std::cout << std::endl;
  // lock nodes
  for (const auto& bit: key) {
    // run fault impact analysis
    FaultImpactAnalysis fia(map);
    fia.run();
    core::Node* node_to_lock = nullptr;
    for (const auto& entry: fia.get_res()) {
      if (entry.first->has_locked) continue;
      if (entry.first->is_lock) continue;
      if (entry.first->is_key_input) continue;
      node_to_lock = entry.first;
      break;
    }
    std::cout << "Picked " << node_to_lock->name << std::endl;
    map.lock_node(node_to_lock, bit);
  }
}

void lock_by_percentage(core::NodeMap& map, float percentage, u_int64_t seed = time(0)) {
  if (percentage < 0.0 || percentage > 1.0) {
    throw std::invalid_argument("percentage must be between 0.0 and 1.0");
  }
  // this conversion is not perfect, but should be good enough
  std::size_t nBits = (std::size_t)std::ceil(map.map.size() * percentage);
  lock_n_gates(map, nBits,seed);
}

}
