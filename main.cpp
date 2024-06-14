#include "parser.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
  if (argc != 3) return 1;
  core::NodeMap map = core::NodeMap();
  map.load(argv[1]);
  map.show();
  map.save(argv[2]);
  return 0;
}
