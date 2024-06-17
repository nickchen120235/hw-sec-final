#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

// Note: `OUTPUT` is a special type which will be changed after reading connection info
#define foreach_gate_type \
  _(0, INPUT, "INPUT", "input") \
  _(1, OUTPUT, "OUTPUT", "output") \
  _(2, NOT, "NOT", "not") \
  _(3, NAND, "NAND", "nand") \
  _(4, AND, "AND", "and") \
  _(5, XNOR, "XNOR", "xnor") \
  _(6, NOR, "NOR", "nor") \
  _(7, XOR, "XOR", "xor") \
  _(8, OR, "OR", "or") \
  _(9, BUF, "BUF", "buf")

#define foreach_gate_type_no_in_out \
  _(2, NOT, "NOT", "not") \
  _(3, NAND, "NAND", "nand") \
  _(4, AND, "AND", "and") \
  _(5, XNOR, "XNOR", "xnor") \
  _(6, NOR, "NOR", "nor") \
  _(7, XOR, "XOR", "xor") \
  _(8, OR, "OR", "or") \
  _(9, BUF, "BUF", "buf")

namespace core {

typedef enum _GateType {
#define _(x, y, z, w) y = x,
  foreach_gate_type
#undef _
} GateType;

// Basic node structure
class Node {
  public:
  // Name of the node
  std::string name;
  // Type of the node
  GateType type;
  // Is output node
  bool is_output;
  // Is lock node
  bool is_lock;
  // Is key input
  bool is_key_input;
  // Has been locked by someone else
  bool has_locked;
  // Input nodes of the node if any
  std::vector<Node*> inputs;
  // Output nodes of the node if any
  std::vector<Node*> outputs;

  Node() { }

  Node(const std::string& name, GateType type) {
    this->name = name;
    this->type = type;
    this->is_output = false;
    this->is_key_input = false;
    this->is_lock = false;
    this->has_locked = false;
    this->inputs.clear();
  }

  void invert() {
    switch (this->type) {
      case GateType::NOT:
        this->type = GateType::BUF;
        break;
      case GateType::BUF:
        this->type = GateType::NOT;
        break;
      case GateType::AND:
        this->type = GateType::NAND;
        break;
      case GateType::NAND:
        this->type = GateType::AND;
        break;
      case GateType::OR:
        this->type = GateType::NOR;
        break;
      case GateType::NOR:
        this->type = GateType::OR;
        break;
      case GateType::XOR:
        this->type = GateType::XNOR;
        break;
      case GateType::XNOR:
        this->type = GateType::XOR;
        break;
      default:
        break;
    }
  }
};

class NodeMap {
  public:
  std::unordered_map<std::string, Node*> map;
  std::vector<Node*> inputs;
  std::vector<Node*> outputs;
  std::vector<Node*> out_gates;
  std::vector<Node*> gates;
  std::vector<Node*> lock_gates;

  NodeMap() { }
  ~NodeMap() {
    for (auto it = map.begin(); it!= map.end(); ++it) {
      delete it->second;
    }
    map.clear();
  }
  /**
   * @brief Get the Node object
   * 
   * @param name Name of the node
   * @return Node* Pointer to the node, or `nullptr` if not found
   */
  inline Node* get_node(const std::string& name) {
    if (map.find(name) == map.end()) return nullptr;
    return map[name];
  }
  /**
   * @brief Add a node to the map
   * 
   * @param node Pointer to `Node` object
   */
  inline void add_node(Node* node) {
    map[node->name] = node;
    switch (node->type) {
      case GateType::INPUT:
        inputs.push_back(node);
        break;
      case GateType::OUTPUT:
        outputs.push_back(node);
        out_gates.push_back(node);
        break;
      #define _(x, y, z, w) \
      case GateType::y: \
      if (node->is_lock) lock_gates.push_back(node); \
      else gates.push_back(node); \
      break;
      foreach_gate_type_no_in_out
      #undef _
      default: break;
    }
  }
  /**
   * @brief Lock a node. This adds a lock node into the circuit
   * 
   * @param node Node to be locked
   * @param key Key bit
   */
  void lock_node(Node* node, bool key);
  /**
   * @brief Load node data from a file
   * 
   * @param filename file to be loaded
   * @param verbose enable debug output, defaults to `false`
   * @throws `std::runtime_error` if the file cannot be opened
   */
  void load(const std::string& filename, bool verbose = false);
  /**
   * @brief Save node data to a file
   * 
   * @param filename file to be saved
   * @param verbose enable debug output, defaults to `false`
   */
  void save(const std::string& filename, bool verbose = false);
  /**
   * @brief Show data stored in the map
   * 
   */
  void show();
};

}
