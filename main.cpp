#include "parser.hpp"
#include "random.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    if (argc != 3)
        return 1;
    core::NodeMap map = core::NodeMap();
    map.load(argv[1]);

    core::Node *node = map.get_node("G1gat");

    if (node) {
        map.lock_node(node, false);
    } else {
        std::cout << "node not found" << std::endl;
        return 1;
    }

    map.show();
    map.save(argv[2]);
    return 0;
}
