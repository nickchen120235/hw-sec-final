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

    std::vector<SLL_Node*> outputs;
    std::vector<SLL_Node*> inputs;
  };

  std::unordered_map<core::Node*, SLL_Node> node_map;

  core::Node final_output;

  void __cut_point(std::unordered_set<SLL_Node*>& graph, SLL_Node* node, int& time,
                   std::unordered_map<SLL_Node*, int>& d, std::unordered_map<SLL_Node*, int>& low,
                   std::unordered_map<SLL_Node*, SLL_Node*>& parent, std::unordered_map<SLL_Node*, bool>& is_cut);

  void dominate_gate(SLL_Node* node);

public:
  SLL(core::NodeMap& node_map);

  void create_node(core::Node* node);

  void Lock(core::NodeMap& node_map);
};