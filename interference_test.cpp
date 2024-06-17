#include "parser.hpp"

#include "interference.hpp"

int main(int argc, char* argv[]) {
  // if (argc != 3) return 1;
  core::NodeMap map = core::NodeMap();
  map.load(argv[1]);

  SLL Locker = SLL(map);

  Locker.pre_initialize();

  return 0;
}
