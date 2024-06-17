#include "interference.hpp"

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

void SLL::dominate_gate(SLL_Node* node) {

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
}

SLL::SLL(core::NodeMap& node_map) {
  final_output.name = "FinalOutput";
  final_output.type = core::GateType::BUF;
  create_node(&final_output);

  // for (std::pair<std::string, core::Node*> p : node_map.map) {
  //   // if (p.second->name == "G371gat") {
  //   //   std::cout << "WTF";
  //   // }
  //   create_node(p.second);
  // }

  for (core::Node* p : node_map.inputs) {
    create_node(p);
  }

  for (core::Node* p : node_map.gates) {
    create_node(p);
  }

  for (core::Node* p : node_map.outputs) {
    create_node(p);
  }

  for (std::pair<core::Node*, SLL_Node> n : this->node_map) {
    if (node_map.map.find(n.second.node->name) == node_map.map.end() && n.second.node->name != "FinalOutput") {
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
  }

  for (core::Node* next : node->outputs) {
    if (node_map.find(next) == node_map.end()) { // if next node not found
      node_map[next] = SLL_Node(next);           // create one
    }

    node_map[node].outputs.push_back(&node_map[next]);
    node_map[next].inputs.push_back(&node_map[node]);
  }

  if (node->is_output) {
    node_map[node].outputs.push_back(&node_map[&final_output]);
    node_map[&final_output].inputs.push_back(&node_map[node]);
  }
}