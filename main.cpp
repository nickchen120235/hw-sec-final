#include "fault.hpp"
#include "options.hpp"
#include "parser.hpp"
#include "random.hpp"
#include "visualization.hpp"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {

  // parse command line arguments

  OptionParser parser;

  parser.parse_arguments(argc, argv);

  core::NodeMap map = core::NodeMap();
  map.load(parser.input_file_name);

  // set seed

  u_int64_t seed = parser.seed_is_set ? parser.seed : time(nullptr);

  // select algorithm
  if (parser.alg == OptionParser::Algorithm::RLL) {

    if (parser.lock_bits != 0)
      RLL::lock_n_gates(map, parser.lock_bits, seed);
    else if(parser.lock_percentage != 0)
      RLL::lock_by_percentage(map, parser.lock_percentage, seed);
  }
  else if (parser.alg == OptionParser::Algorithm::FLL) {
    if (parser.lock_bits != 0)
      FLL::lock_n_gates(map, parser.lock_bits, parser.FLL_rounds, seed);
    else if(parser.lock_percentage != 0)
      FLL::lock_by_percentage(map, parser.lock_percentage, parser.FLL_rounds, seed);
  }

  Visualization::write_to_verilog_file(map, parser.visualization_file_name, parser.show_intermediate_gates);

  map.save(parser.output_file_name);
  return 0;
}
