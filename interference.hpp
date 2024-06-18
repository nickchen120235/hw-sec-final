#pragma once
#include "parser.hpp"
#include <list>
#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
class SLL {

private:
  class SLL_Node {
  public:
    SLL_Node(core::Node*);
    SLL_Node() {};

    core::Node* node;

    int rank = 0;

    std::vector<SLL_Node*> dominates;

    std::vector<SLL_Node*> outputs;
    std::vector<SLL_Node*> inputs;

    bool processed = false;
    int input_gates = 0;
    int converge = 0;
  };

  std::unordered_map<core::Node*, SLL_Node> node_map;
  std::vector<SLL_Node*> nodes;

  core::Node final_output;

  void __cut_point(std::unordered_set<SLL_Node*>& graph, SLL_Node* node, int& time,
                   std::unordered_map<SLL_Node*, int>& d, std::unordered_map<SLL_Node*, int>& low,
                   std::unordered_map<SLL_Node*, SLL_Node*>& parent, std::unordered_map<SLL_Node*, bool>& is_cut);

  void __find_dominate_gate(SLL_Node* node);

  void create_node(core::Node* node);

  void calculate_converge();

  void find_dominate_gate();

public:
  SLL(core::NodeMap& node_map);

  void pre_initialize();

  std::vector<SLL::SLL_Node*> initialize();

  void Lock(core::NodeMap& node_map);
};