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

void NodeMap::lockNode(Node* node, bool key) {
  if (node->isLock)
    throw std::runtime_error("Cannot lock a lock node");
  // create key input node
  Node* keyInput = new Node();
  keyInput->type = GateType::INPUT;
  keyInput->name = std::string("keyinput") + std::to_string(this->lockGates.size());
  keyInput->isOutput = false;
  keyInput->isLock = false;
  this->addNode(keyInput);
  // create lock node
  Node* lock = new Node();
  lock->type = key ? GateType::XNOR : GateType::XOR;
  lock->name = node->name + "$enc";
  lock->isOutput = false;
  lock->isLock = true;
  lock->inputs.push_back(keyInput);
  lock->inputs.push_back(node);
  this->addNode(lock);
  // replace the original node with the lock node
  for (auto&& gate: this->gates) {
    std::replace_if(gate->inputs.begin(), gate->inputs.end(), [&node](Node* n){ return n == node; }, lock);
  }
}

void NodeMap::load(const std::string& filename, bool verbose) {
  std::cout << "Loading " << filename << std::endl;
  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file " + filename);
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
      Node* node = new Node();
      node->type = GateType::INPUT;
      node->name = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
      node->isOutput = false;
      verbose && std::cout << "Name: " << node->name << std::endl;
      this->addNode(node);
    }
    else if (line.rfind("OUTPUT", 0) == 0) {
      verbose && std::cout << "Type: OUTPUT" << std::endl;
      Node* node = new Node();
      node->type = GateType::OUTPUT;
      node->name = line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1);
      node->isOutput = true;
      verbose && std::cout << "Name: " << node->name << std::endl;
      this->addNode(node);
    }
    else if (line.find('=') != std::string::npos) {
      bool isNewNode = false;
      std::string name = line.substr(0, line.find('=') - 1);
      trim(name);
      verbose && std::cout << "Name: " << name << std::endl;
      Node* node = this->getNode(name);
      if (node != nullptr) {
        verbose && std::cout << "Found existing node \"" << name << "\"" << std::endl;
      }
      else {
        node = new Node();
        node->name = name;
        node->isOutput = false;
        isNewNode = true;
      }
      if (0);
      #define _(x, y, z, w) else if (line.find(z, line.find('=') + 1) != std::string::npos || line.find(w, line.find('=') + 1) != std::string::npos) { \
        verbose && std::cout << "Type: " << z << std::endl; \
        node->type = GateType::y; \
      }
      foreach_gate_type
      #undef _
      if (isNewNode) this->addNode(node);
      std::stringstream ss(line.substr(line.find('(') + 1, line.find(')') - line.find('(') - 1));
      while (ss.good()) {
        std::string input;
        std::getline(ss, input, ',');
        trim(input);
        Node* inputNode = this->getNode(input);
        if (inputNode != nullptr) {
          verbose && std::cout << "Input: " << input << std::endl;
          node->inputs.push_back(inputNode);
        }
        else {
          std::cerr << "Encountered unknown input: " << input << std::endl;
        }
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
    throw std::runtime_error("Could not open file " + filename);
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
  for (const auto& node: this->outGates) {
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
  for (const auto& node: this->lockGates) {
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
  file.close();
  std::cout << "Done. Saved " << this->inputs.size() << " inputs, "
            << this->outputs.size() << " outputs, and "
            << this->gates.size() + this->lockGates.size() << " intermediate gates." << std::endl;
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
    if (entry.second->isOutput) {
      std::cout << "Output" << std::endl;
    }
    if (entry.second->isLock) {
      std::cout << "Lock" << std::endl;
    }
    if (entry.second->type != GateType::INPUT) {
      std::cout << "Inputs: ";
      for (const auto& input: entry.second->inputs) {
        std::cout << input->name << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
}

}
