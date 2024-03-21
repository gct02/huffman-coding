#pragma once

#include <iostream>
#include <fstream>
#include <string>

class RAIIFileHandler 
{
private:
	// Alias declarations
	using ios = std::ios;
	using fstream = std::fstream;
	using string = std::string;

public:
	RAIIFileHandler(const string& path, ios::openmode openMode = DEFAULT_OPEN_MODE)
		: file(path, openMode)
	{
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open file: " + path);
		}

		file.rdbuf()->pubsetbuf(buffer, BUFFER_SIZE);
	}

	~RAIIFileHandler() 
	{
		if (file.is_open()) {
			file.close();
		}
	}

	bool isOpen() const 
	{
		return file.is_open();
	}

	fstream& get()
	{
		return file;
	}

private:
	static constexpr ios::openmode DEFAULT_OPEN_MODE = ios::binary | ios::in | ios::out;
	static constexpr size_t BUFFER_SIZE = 1024;

	fstream file;
	char buffer[BUFFER_SIZE];
};