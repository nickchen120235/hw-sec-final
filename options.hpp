#include <iostream>
#include <string.h>
#include <string>

class OptionParser {
public:
  enum ArgError {
    MISSING_ARG = 0,
    UNKNOW_INPUT = 1,
    INVALID_ARG = 2,
    INVALID_FILE_NAME = 3,
    INVALID_INPUT = 4,
    CONFLICT_ARG = 5,
  };

  enum Algorithm {
    RLL = 0,
    FLL = 1,
  };

  Algorithm alg = Algorithm::RLL;

  bool show_help = false;
  int lock_bits = 0;
  float lock_percentage = 0.0;
  u_int32_t FLL_rounds = 1000;
  u_int64_t seed = 0;
  bool seed_is_set = false;
  std::string input_file_name = "input.bench";
  std::string output_file_name = "output.bench";
  std::string visualization_file_name = "output.v";

  void parse_arguments(int argc, char* argv[]) {

    // clang-format off
#define option_cmp(argv, option) strncmp(argv, option, sizeof(option)) == 0
#define i_plus_1_with_check if (i+1 >= argc) {show_error_and_exit(argc, argv, i, ArgError::MISSING_ARG);} else {i++;};
#define check_invalid_arg_and_exit if (argv[i][0] == '-') {show_error_and_exit(argc, argv, i - 1, ArgError::MISSING_ARG);} else {show_error_and_exit(argc, argv, i, ArgError::UNKNOW_INPUT);}
    // clang-format on

    for (int i = 1; i < argc; i++) {

      if (option_cmp(argv[i], "-a") || option_cmp(argv[i], "--algorithm")) {

        i_plus_1_with_check;

        if (option_cmp(argv[i], "RLL")) {
          alg = Algorithm::RLL;
        }
        else if (option_cmp(argv[i], "FLL")) {
          alg = Algorithm::FLL;
        }
        else {
          check_invalid_arg_and_exit;
        }
      }
      else if (option_cmp(argv[i], "-b") || option_cmp(argv[i], "--lock-by-bits")) {
        lock_bits = 1; // to test conflict

        if (conflict_happen()) {
          show_error_and_exit(argc, argv, i, ArgError::CONFLICT_ARG);
        }

        i_plus_1_with_check;

        try {
          lock_bits = strtol(argv[i], 0, 10);
        } catch (std::invalid_argument& e) {
          show_error_and_exit(argc, argv, i, ArgError::INVALID_INPUT);
        }

        if (lock_bits <= 0) {
          show_error_and_exit(argc, argv, i, ArgError::INVALID_INPUT);
        }
      }
      else if (option_cmp(argv[i], "-h") || option_cmp(argv[i], "--help")) {
        show_help = true;
      }
      else if (option_cmp(argv[i], "-i") || option_cmp(argv[i], "--input-file")) {

        i_plus_1_with_check;

        if (argv[i][0] == '-') {
          show_error_and_exit(argc, argv, i, ArgError::MISSING_ARG);
        }

        input_file_name = argv[i];
      }
      else if (option_cmp(argv[i], "-o") || option_cmp(argv[i], "--output-file")) {

        i_plus_1_with_check;

        if (argv[i][0] == '-') {
          show_error_and_exit(argc, argv, i, ArgError::MISSING_ARG);
        }

        output_file_name = argv[i];
      }
      else if (option_cmp(argv[i], "-p") || option_cmp(argv[i], "--lock-by-percentage")) {
        lock_percentage = 0.1; // to test conflict

        if (conflict_happen()) {
          show_error_and_exit(argc, argv, i, ArgError::CONFLICT_ARG);
        }

        i_plus_1_with_check;

        try {
          lock_percentage = strtod(argv[i], 0);
        } catch (std::invalid_argument& e) {
          show_error_and_exit(argc, argv, i, ArgError::INVALID_INPUT);
        }

        if (lock_percentage <= 0 || lock_percentage > 1) {
          show_error_and_exit(argc, argv, i, ArgError::INVALID_INPUT);
        }
      }
      else if (option_cmp(argv[i], "-r") || option_cmp(argv[i], "--rounds")) {

        i_plus_1_with_check;

        try {
          if (argv[i][0] == '-') { // ignore negative number
            show_error_and_exit(argc, argv, i, ArgError::INVALID_INPUT);
          }
          FLL_rounds = strtoul(argv[i], 0, 10);
        } catch (std::invalid_argument& e) {
          show_error_and_exit(argc, argv, i, ArgError::INVALID_INPUT);
        }

        if (FLL_rounds <= 0) {
          show_error_and_exit(argc, argv, i, ArgError::INVALID_INPUT);
        }
      }
      else if (option_cmp(argv[i], "-s") || option_cmp(argv[i], "--seed")) {
        i_plus_1_with_check;

        try {
          seed = strtoull(argv[i], 0, 10);
          seed_is_set = true;
        } catch (std::invalid_argument& e) {
          show_error_and_exit(argc, argv, i, ArgError::INVALID_INPUT);
        }
      }
      else if (option_cmp(argv[i], "-v") || option_cmp(argv[i], "--visualization-file")) {

        i_plus_1_with_check;

        if (argv[i][0] == '-') {
          show_error_and_exit(argc, argv, i, ArgError::MISSING_ARG);
        }

        visualization_file_name = argv[i];
      }
      else {
        show_error_and_exit(argc, argv, i, ArgError::UNKNOW_INPUT);
      }
    }

#undef option_cmp

    process_args();
  }

  void show_error_and_exit(int argc, char* argv[], int current, ArgError error) {

    for (int i = 0; i < argc; i++) {
      std::cout << argv[i] << " ";
    }

    std::cout << std::endl;

    for (int i = 0; i < argc; i++) {
      if (i != current) {
        for (std::size_t j = 0; j < strlen(argv[i]); j++) {
          std::cout << " ";
        }
        std::cout << " ";
      }
      else if (error == ArgError::MISSING_ARG) {
        for (std::size_t j = 0; j < strlen(argv[i]); j++) {
          std::cout << " ";
        }
        std::cout << "^";
      }
      else {
        for (std::size_t j = 0; j < strlen(argv[i]); j++) {
          std::cout << "^";
        }
        std::cout << " ";
      }
    }

    std::cout << std::endl;

    switch (error) {
    case ArgError::UNKNOW_INPUT:
      std::cout << "Unknown input: " << argv[current] << std::endl;
      break;
    case ArgError::INVALID_ARG:
      std::cout << "Invalid argument: " << argv[current] << std::endl;
      break;
    case ArgError::MISSING_ARG:
      std::cout << "Missing argument: " << argv[current] << std::endl;
      break;
    case ArgError::INVALID_FILE_NAME:
      std::cout << "Invalid file name: " << argv[current] << std::endl;
      break;
    case ArgError::INVALID_INPUT:
      std::cout << "Invalid input: " << argv[current] << std::endl;
      break;
    case ArgError::CONFLICT_ARG:
      std::cout << "conflict argument: " << argv[current] << std::endl;
      break;
    }

    std::cout << std::endl;

    print_help();

    exit(1);
  }

  bool conflict_happen() {
    return (lock_bits != 0 && lock_percentage != 0) || (lock_bits == 0 && lock_percentage == 0);
  }

  void process_args() {
    if (show_help) {
      print_help();
      exit(0);
    }

    if (lock_bits == 0 && lock_percentage == 0) {
      std::cout<< "Error: -b or -p is required" << std::endl<<std::endl;
      print_help();
      exit(1);
    }
  }

  // clang-format off
  void print_help() {

    std::cout << "Hardware Security Final Project" << std::endl;
    std::cout << "Usage: " << std::endl;
    std::cout << "  -a, --algorithm <RLL | FLL>             select Locking algorithm. (default: RLL)" << std::endl;
    std::cout << "  -b, --lock-by-bits <N>                  REQUIRED. conflict with -p. number of bits to lock (N > 0)" << std::endl;
    std::cout << "  -h, --help                              print help message" << std::endl;
    std::cout << "  -i, --input-file <filename>             input file name. (default: input.bench)" << std::endl;
    std::cout << "  -o, --output-file <filename>            output file name. (default: output.bench)" << std::endl;
    std::cout << "  -p, --lock-by-percentage <N>            REQUIRED. conflict with -b. percentage to lock (0.0 < N <= 1.0)" << std::endl;
    std::cout << "  -r, --rounds <N>                        test rounds for one lock bit in FLL algorithm. (default 1000)" << std::endl;
    std::cout << "                                          This option only takes effect when algorithm is set to FLL" << std::endl;
    std::cout << "  -s, --seed <N>                          seed for random number generator. (default: time(0))" << std::endl;
    std::cout << "  -v, --visualization-file <filename>     output file name for visualization. (default: output.v)" << std::endl;

    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  ./main -a FLL -i 1000 -s 123456 -b 10 -o output.bench -v output.v" << std::endl;
    std::cout << "  ./main -a FLL -i 1000 -s 123456 -p 0.123 -o output.bench -v output.v" << std::endl;
  }

  // clang-format on
};