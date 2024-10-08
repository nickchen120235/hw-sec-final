#include "parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> 
#include <cctype>
#include <locale>

// helper functions
// trim from start (in place)
inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
    rtrim(s);
    ltrim(s);
}

namespace core {

void NodeMap::lock_node(Node* node, bool key) {
  if (node->is_lock)
    throw std::runtime_error("Cannot lock a lock node");
  // create key input node
  Node* keyInput = new Node(std::string("keyinput") + std::to_string(this->_lock_gates.size()), GateType::INPUT);
  keyInput->is_output = false;
  keyInput->is_lock = false;
  keyInput->is_key_input = true;
  this->add_node(keyInput);
  /**
   * create the lock gate
   * 
   * first we randomly choose between XOR and XNOR
   * if the key is 0 and the lock gate is XNOR, we invert the lock gate
   * if the key is 1 and the lock gate is XOR, we invert the lock gate
   * otherwise, we leave the lock gate as is
   */
  Node* lock = new Node(node->name + "$enc", std::rand() % 2 == 0 ? GateType::XOR : GateType::XNOR);
  bool invert = (key == 0 && lock->type == GateType::XNOR) || (key == 1 && lock->type == GateType::XOR);
  lock->is_lock = true;
  this->add_node(lock);
  if (node->type == GateType::INPUT) {
    if (invert) {
      Node* inv = new Node(node->name + "$inv", GateType::NOT);
      this->add_node(inv);
      inv->is_lock = true;
      inv->inputs.push_back(node);
      lock->inputs.push_back(keyInput);
      lock->inputs.push_back(inv);
    }
    else {
      lock->inputs.push_back(node);
      lock->inputs.push_back(keyInput);
    }
  }
  else {
    lock->inputs.push_back(node);
    lock->inputs.push_back(keyInput);
    if (invert) node->invert();
  }
  // if node is an output, replace the original node with the lock node
  if (node->is_output) {
    std::replace_if(this->outputs.begin(), this->outputs.end(), [&node](Node* n){ return n == node; }, lock);
  }
  // replace the original node with the lock node
  for (auto&& gate: this->map) {
    if (gate.second->is_lock) continue;
    std::replace_if(gate.second->inputs.begin(), gate.second->inputs.end(), [&node](Node* n){ return n == node; }, lock);
  }
  node->has_locked = true;
}

void NodeMap::load(const std::string& filename, bool verbose) {
  std::cout << "Loading " << filename << std::endl;
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout<< "Could not open file " + filename << std::endl;
    exit(1);
  }
  std::string line;
  while (std::getline(file, line)) {
    trim(line);
    if (line.length() == 0) {
      continue;
    }
    verbose && std::cout << "Parsing: " << line << std::endl;

    if (line.rfind("INPUT", 0) == 0) {
      verbose && std::cout << "Type: INPUT" << std::endl;
      Node* node = new Node(line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1), GateType::INPUT);
      node->is_output = false;
      verbose && std::cout << "Name: " << node->name << std::endl;
      this->add_node(node);
    }
    else if (line.rfind("OUTPUT", 0) == 0) {
      verbose && std::cout << "Type: OUTPUT" << std::endl;
      Node* node = new Node(line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1), GateType::OUTPUT);
      node->is_output = true;
      verbose && std::cout << "Name: " << node->name << std::endl;
      this->add_node(node);
    }
    else if (line.find('=') != std::string::npos) {
      bool isNewNode = false;
      std::string name = line.substr(0, line.find('=') - 1);
      trim(name);
      verbose && std::cout << "Name: " << name << std::endl;
      Node* node = this->get_node(name);
      if (node != nullptr) {
        verbose && std::cout << "Found existing node \"" << name << "\"" << std::endl;
      }
      else {
        node = new Node();
        node->name = name;
        node->is_output = false;
        node->is_lock = false;
        isNewNode = true;
      }
      if (0);
      #define _(x, y, z, w) else if (line.find(z, line.find('=') + 1) != std::string::npos || line.find(w, line.find('=') + 1) != std::string::npos) { \
        verbose && std::cout << "Type: " << z << std::endl; \
        node->type = GateType::y; \
      }
      foreach_gate_type_no_in_out
      #undef _
      if (isNewNode) this->add_node(node);
      std::stringstream ss(line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1));
      while (ss.good()) {
        std::string input;
        std::getline(ss, input, ',');
        trim(input);
        Node* inputNode = this->get_node(input);
        if (inputNode != nullptr) {
          verbose && std::cout << "Input: " << input << std::endl;
        }
        else {
          inputNode = new Node(input, GateType::OUTPUT); // using OUTPUT as a dummy
          node->is_output = false;
          node->is_lock = false;
        }
        node->inputs.push_back(inputNode);
        inputNode->outputs.push_back(node);
      }
    }
    else {
      std::cerr << "Encountered unknown line: " << line << std::endl;
    }
  }
  file.close();
  std::cout << "Done. Loaded " << this->inputs.size() << " inputs, "
            << this->outputs.size() << " outputs, and "
            << this->gates.size() << " intermediate gates." << std::endl;
}

void NodeMap::save(const std::string& filename, bool verbose) {
  std::cout << "Saving " << filename << std::endl;
  std::ofstream file(filename);
  if (!file.is_open()) {
    std::cout<< "Could not open file " + filename << std::endl;
    exit(1);
  }
  for (const auto& node: this->inputs) {
    verbose && std::cout << "Writing INPUT(" << node->name << ")" << std::endl;
    file << "INPUT(" << node->name << ")" << std::endl;
  }
  for (const auto& node: this->outputs) {
    verbose && std::cout << "Writing OUTPUT(" << node->name << ")" << std::endl;
    file << "OUTPUT(" << node->name << ")" << std::endl;
  }
  file << std::endl;
  for (const auto& node: this->gates) {
    std::string logicInputs;
    for (const auto& input: node->inputs) logicInputs += input->name + ", ";
    // remove trailing comma
    logicInputs.pop_back(); logicInputs.pop_back();
    switch (node->type) {
      #define _(x, y, z, w) case GateType::y: file << node->name << " = " << z << "(" << logicInputs << ")" << std::endl; break;
      foreach_gate_type_no_in_out
      #undef _
      default:
        file << node->name << " = UNKNOWN(" << logicInputs << ")" << std::endl;
        break;
    }
  }
  file << std::endl;
  file.close();
  std::cout << "Done. Saved " << this->inputs.size() << " inputs, "
            << this->outputs.size() << " outputs, and "
            << this->gates.size() << " intermediate gates." << std::endl;
}

void NodeMap::show() {
  for (const auto& entry: this->map) {
    std::cout << "Name: " << entry.first << std::endl;
    std::cout << "Type: ";
    switch (entry.second->type) {
      #define _(x, y, z, w) case GateType::y: std::cout << z << std::endl; break;
      foreach_gate_type
      #undef _
      default:
        std::cout << "UNKNOWN" << std::endl;
        break;
    }
    if (entry.second->is_output) {
      std::cout << "Output" << std::endl;
    }
    if (entry.second->is_lock) {
      std::cout << "Lock" << std::endl;
    }
    if (entry.second->type != GateType::INPUT) {
      std::cout << "Inputs: ";
      for (const auto& input: entry.second->inputs) {
        std::cout << input->name << " ";
      }
      std::cout << std::endl;
    }
    std::cout << "Outputs: ";
    for (const auto& output: entry.second->outputs) {
      std::cout << output->name << " ";
    }
    std::cout << std::endl;
  }
}

}
