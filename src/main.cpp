#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <stdexcept>
#include <algorithm>

#include "huffman.hpp"
#include "path-manager.h"
#include "messages.h"

namespace fs = std::filesystem;

// Constants
static const std::string ZIP_CMD = "zip";
static const std::string UNZIP_CMD = "unzip";
static const std::string ZIPPED_EXT = ".hzip";

enum Operation { ZIP = 1, UNZIP = 2 };

// Function prototypes
bool isValidCommandLineArgs(int argc, const std::string& command);
int processCommandLineArgs(int argc, char** argv);
int promptUserForOperation();
int compressFile();
int decompressFile();

int main(int argc, char** argv) {
	if (argc > 1) {
		// If there is command line arguments, process them 
		// and run the application automatically
		try {
			return processCommandLineArgs(argc, argv);
		}
		catch (std::invalid_argument& e) {
			std::cout << e.what() << std::endl;
			return 1;
		}
	}
	else {
		// If there is no command line arguments, prompt the user to 
		// provide the desired operation and the input / output file paths 
		return promptUserForOperation();
	}
}

// It supposes that the command line arguments are correct
int processCommandLineArgs(int argc, char** argv) {
	std::string command(argv[1]);
	std::transform(command.begin(), command.end(), command.begin(), ::tolower);

	if (!isValidCommandLineArgs(argc, command)) {
		throw std::invalid_argument(Messages::INVALID_ARGUMENTS);
	}

	std::string inputFilePath(argv[2]);

	if (!fs::exists(inputFilePath)) {
		std::cout << "Error: The specified input file does not exist." << std::endl;
		return 1;
	}

	std::string outputFilePath;

	if (argc == 4) {
		// Get provided output file path if the user provided one
		outputFilePath = argv[3];

		// Put a default extension (".hzip") in the compressed file's path 
		// if the user didn't provide one
		if (!hasExtension(outputFilePath)) {
			outputFilePath += ZIPPED_EXT;
		}
	}
	else {
		// If the user didn't provided an output file path, the compressed file will 
		// be created with the same path as the input file, but with a ".hzip" extension
		outputFilePath = stripExtension(inputFilePath) + ZIPPED_EXT;
	}

	if (command == ZIP_CMD) {
		Compressor::zip(inputFilePath, outputFilePath);
		std::cout << "File compressed successfully!" << std::endl;
	} 
	else {
		Decompressor::unzip(inputFilePath, outputFilePath);
		std::cout << "File decompressed successfully!" << std::endl;
	}

	return 0;
}

bool isValidCommandLineArgs(int argc, const std::string& command) {
	return !(argc > 4 || argc < 3 ||
		!(command == ZIP_CMD || command == UNZIP_CMD) ||
		(command == UNZIP_CMD && argc != 4));
}

int promptUserForOperation() {
	std::cout << "Please choose the desired operation:" << std::endl;
	std::cout << "1. Compress file" << std::endl;
	std::cout << "2. Decompress file" << std::endl;

	int choice;
	std::cin >> choice;

	switch (choice) {
	case ZIP:
		return compressFile();
	case UNZIP:
		return decompressFile();
	default:
		std::cout << "Error: Invalid operation selected." << std::endl;
		return 1;
	}
}

int compressFile() {
	std::cout << "Please provide the path to the file you want to compress: ";
	std::string filePath;
	std::cin >> filePath;

	if (!fs::exists(filePath)) {
		std::cout << "Error: The specified file does not exist." << std::endl;
		return 1;
	}

	std::cout << "Please provide the path where the compressed file should be created: ";
	std::string compressedFilePath;
	std::cin >> compressedFilePath;

	if (!hasExtension(compressedFilePath)) {
		compressedFilePath += ZIPPED_EXT;
	}

	Compressor::zip(filePath, compressedFilePath);
	std::cout << "File compressed successfully!" << std::endl;
	return 0;
}

int decompressFile() {
	std::cout << "Please provide the path to the file you want to decompress: ";
	std::string compressedFilePath;
	std::cin >> compressedFilePath;

	if (!fs::exists(compressedFilePath)) {
		std::cout << "Error: The specified file does not exist." << std::endl;
		return 1;
	}

	std::cout << "Please provide the path where the decompressed file should be created: ";
	std::string decompressedFilePath;
	std::cin >> decompressedFilePath;

	Decompressor::unzip(compressedFilePath, decompressedFilePath);
	std::cout << "File decompressed successfully!" << std::endl;
	return 0;
}