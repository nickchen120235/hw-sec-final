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
typedef struct _Node {
  // Name of the node
  std::string name;
  // Type of the node
  GateType type;
  // Is output node
  bool isOutput;
  // Is lock node
  bool isLock;
  // Input nodes of the node if any
  std::vector<struct _Node*> inputs;
} Node;

class NodeMap {
  std::unordered_map<std::string, Node*> map;
  public:
  std::vector<Node*> inputs;
  std::vector<Node*> outputs;
  std::vector<Node*> outGates;
  std::vector<Node*> gates;
  std::vector<Node*> lockGates;

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
  inline Node* getNode(const std::string& name) {
    if (map.find(name) == map.end()) return nullptr;
    return map[name];
  }
  /**
   * @brief Add a node to the map
   * 
   * @param node Pointer to `Node` object
   */
  inline void addNode(Node* node) {
    map[node->name] = node;
    switch (node->type) {
      case GateType::INPUT:
        inputs.push_back(node);
        break;
      case GateType::OUTPUT:
        outputs.push_back(node);
        outGates.push_back(node);
        break;
      #define _(x, y, z, w) \
      case GateType::y: \
      if (node->isLock) lockGates.push_back(node); \
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
  void lockNode(Node* node, bool key);
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
