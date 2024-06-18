#include "interference.hpp"

#include <algorithm>
#include <assert.h>

SLL::SLL_Node::SLL_Node(core::Node* node) {
  this->node = node;
}

void SLL::__cut_point(std::unordered_set<SLL_Node*>& graph, SLL_Node* node, int& time,
                      std::unordered_map<SLL_Node*, int>& d, std::unordered_map<SLL_Node*, int>& low,
                      std::unordered_map<SLL_Node*, SLL_Node*>& parent, std::unordered_map<SLL_Node*, bool>& is_cut) {

  time++;
  d[node] = time;
  low[node] = time;

  for (SLL_Node* v : node->outputs) {

    if (graph.find(v) != graph.end()) { // next point is in graph
      if (d[v] == 0) {                  // not visited
        parent[v] = node;

        __cut_point(graph, v, time, d, low, parent, is_cut);

        low[node] = std::min(low[node], low[v]);

        if (low[v] >= d[node]) {
          is_cut[node] = true;
        }
      }
      else if (parent[node] != v) {
        low[node] = std::min(low[node], d[v]);
      }
    }
  }

  for (SLL_Node* v : node->inputs) { // pretend this is a undirected graph

    if (graph.find(v) != graph.end()) { // next point is in graph
      if (d[v] == 0) {                  // not visited
        parent[v] = node;

        __cut_point(graph, v, time, d, low, parent, is_cut);

        low[node] = std::min(low[node], low[v]);

        if (parent[node] != nullptr && low[v] >= d[node]) {
          is_cut[node] = true;
        }
      }
      else if (parent[node] != v) {
        low[node] = std::min(low[node], d[v]);
      }
    }
  }
}

void SLL::__find_dominate_gate(SLL_Node* node) {

  std::unordered_set<SLL_Node*> sub_graph;

  // first generate subgraph

  {
    std::queue<SLL_Node*> q;

    q.push(node);
    sub_graph.insert(node);

    SLL_Node* cur;

    while (q.size() > 0) {
      cur = q.front();
      q.pop();

      for (SLL_Node* next : cur->outputs) {
        if (sub_graph.find(next) == sub_graph.end()) { // next point is not in graph
          sub_graph.insert(next);
          q.push(next);
        }
      }
    }
  }

  // Next find dominate gates

  std::unordered_map<SLL_Node*, int> d;
  std::unordered_map<SLL_Node*, int> low;
  std::unordered_map<SLL_Node*, SLL_Node*> parent;
  std::unordered_map<SLL_Node*, bool> is_cut;

  int time = 0;

  for (SLL_Node* n : sub_graph) {
    d[n] = 0;
    low[n] = 0;
    parent[n] = nullptr;
    is_cut[n] = false;
  }

  __cut_point(sub_graph, node, time, d, low, parent, is_cut);

  for (SLL_Node* n : sub_graph) {
    if (is_cut[n]) {
      node->dominates.push_back(n);
    }
  }
}

void SLL::find_dominate_gate() {

  for (SLL_Node* n : this->nodes) {
    __find_dominate_gate(n);
  }
}

SLL::SLL(core::NodeMap& node_map) {

  final_output.name = "FinalOutput";
  final_output.type = core::GateType::BUF;
  create_node(&final_output);

  for (core::Node* p : node_map.inputs) {
    create_node(p);
  }

  for (core::Node* p : node_map.gates) {
    create_node(p);
  }

  for (core::Node* p : node_map.outputs) {
    create_node(p);
  }

  for (SLL_Node* n : this->nodes) {
    if (node_map.map.find(n->node->name) == node_map.map.end() && n->node->name != "FinalOutput") {
      assert(false);
    }
  }

  // Check graph is correctly created
  // {
  //   assert(node_map.map.size() + 1 == this->node_map.size());

  //   for (std::pair<std::string, core::Node*> p : node_map.map) {

  //     // check input/output size
  //     if (p.second->is_output) {
  //       assert(p.second->outputs.size() + 1 == this->node_map[p.second].outputs.size());
  //     }
  //     else {
  //       assert(p.second->outputs.size() == this->node_map[p.second].outputs.size());
  //     }

  //     // check prev nodes

  //     assert(p.second->inputs.size() == this->node_map[p.second].inputs.size());

  //     std::size_t cnt = 0;
  //     for (core::Node* n : p.second->inputs) {

  //       bool found = false;

  //       for (SLL_Node* sn : this->node_map[p.second].inputs) {
  //         if (sn->node == n) {
  //           found = true;
  //           cnt++;
  //           break;
  //         }
  //       }

  //       assert(found);
  //     }
  //     assert(cnt == this->node_map[p.second].inputs.size());

  //     // check next nodes

  //     cnt = 0;
  //     for (core::Node* n : p.second->outputs) {

  //       bool found = false;

  //       for (SLL_Node* sn : this->node_map[p.second].outputs) {
  //         if (sn->node == n) {
  //           found = true;
  //           cnt++;
  //           break;
  //         }
  //       }

  //       assert(found);
  //     }

  //     if (p.second->is_output) {
  //       assert(cnt + 1 == this->node_map[p.second].outputs.size());
  //     }
  //     else {
  //       assert(cnt == this->node_map[p.second].outputs.size());
  //     }
  //   }
  // }
}

void SLL::create_node(core::Node* node) {

  if (node_map.find(node) == node_map.end()) { // if current node not found
    node_map[node] = SLL_Node(node);           // create one
    this->nodes.push_back(&node_map[node]);
  }

  for (core::Node* next : node->outputs) {
    if (node_map.find(next) == node_map.end()) { // if next node not found
      node_map[next] = SLL_Node(next);           // create one
      this->nodes.push_back(&node_map[next]);
    }

    node_map[node].outputs.push_back(&node_map[next]);
    node_map[next].inputs.push_back(&node_map[node]);

    if (!(node->is_output || node->type == core::GateType::INPUT)) {
      node_map[next].input_gates++;
    }
  }

  if (node->is_output) {
    node_map[node].outputs.push_back(&node_map[&final_output]);
    node_map[&final_output].inputs.push_back(&node_map[node]);
  }
}

void SLL::calculate_converge() {

  for (SLL_Node* node : this->nodes) {

    if (!node->node->is_output) {
      for (SLL_Node* next : node->outputs) {
        node->converge = next->input_gates - 1;
      }
    }
  }
}

void SLL::pre_initialize() {

  calculate_converge();
  find_dominate_gate();
}

std::vector<SLL::SLL_Node*> SLL::initialize() {

  std::vector<SLL::SLL_Node*> rankList;

  for (SLL_Node* n : this->nodes) {
    if (!n->processed) {
      rankList.push_back(n);
      std::cout << n->node->name << " " << n->converge << std::endl;
    }
  }

  if (rankList.size() == 0) {
    return rankList;
  }

  std::cout << "====================" << std::endl;

  std::sort(rankList.begin(), rankList.end(),
            [](SLL::SLL_Node* a, SLL::SLL_Node* b) { return a->converge > b->converge; });

  for (SLL::SLL_Node* n : rankList) {
    std::cout << n->node->name << " " << n->converge << std::endl;
  }
}