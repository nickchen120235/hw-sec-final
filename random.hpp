#include "parser.hpp"
#include <algorithm>
#include <iostream>
#include <cmath>

// Random Logic Locking
namespace RLL {

void _lock(core::NodeMap& map, std::vector<core::Node*>& choice, std::size_t keyBits) {
  std::cout << "Locking using Random Logic Locking" << std::endl;
  // prepare key
  std::srand(time(NULL));
  std::size_t nBits = std::min(keyBits, choice.size());
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
  for (std::size_t i = 0; i < key.size(); ++i) {
    map.lock_node(choice[i], key[i]);
  }
}

/**
 * @brief Lock the circuit with `keyBits` bits
 * 
 * @param map Loaded circuit
 * @param keyBits Number of bits of the key
 */
void lock_n_gates(core::NodeMap& map, std::size_t keyBits) {
  // prepare lockable nodes
  std::vector<core::Node*> choice;
  choice.insert(choice.end(), map.inputs.begin(), map.inputs.end());
  choice.insert(choice.end(), map.outputs.begin(), map.outputs.end());
  choice.insert(choice.end(), map.gates.begin(), map.gates.end());
  std::random_shuffle(choice.begin(), choice.end());
  RLL::_lock(map, choice, keyBits);
}

/**
 * @brief Lock the circuit by percentage
 * 
 * @param map Loaded circuit
 * @param percentage Percentage of lockable nodes
 */
void lock_by_percentage(core::NodeMap& map, float percentage) {
  if (percentage < 0.0 || percentage > 1.0) {
    throw std::invalid_argument("percentage must be between 0.0 and 1.0");
  }
  // prepare lockable nodes
  std::vector<core::Node*> choice;
  choice.insert(choice.end(), map.inputs.begin(), map.inputs.end());
  choice.insert(choice.end(), map.outputs.begin(), map.outputs.end());
  choice.insert(choice.end(), map.gates.begin(), map.gates.end());
  std::random_shuffle(choice.begin(), choice.end());
  // this conversion is not perfect, but should be good enough
  std::size_t nBits = (std::size_t)std::ceil(choice.size() * percentage);
  RLL::_lock(map, choice, nBits);
}

}
