#include "messages.h"

namespace Messages 
{
const std::string INVALID_COMMAND =     "Invalid command line arguments.\n";
const std::string USAGE =               "Usage: huffman <command> <input_file> [<output_file>]\n";
const std::string OPTIONS_COMMAND =     "  <command>       Specify the operation to perform: \"zip\" for compression or \"unzip\" for decompression.\n";
const std::string OPTIONS_INPUT_FILE =  "  <input_file>    Path to the file to be processed.\n";
const std::string OPTIONS_OUTPUT_FILE = "  [<output_file>] Path to the resulting file. Optional for \"zip\" operation; required for \"unzip\" operation.\n";
const std::string OPTIONS = "\nOptions:\n" + OPTIONS_COMMAND + OPTIONS_INPUT_FILE + OPTIONS_OUTPUT_FILE;
const std::string INVALID_ARGUMENTS = INVALID_COMMAND + USAGE + OPTIONS;
}