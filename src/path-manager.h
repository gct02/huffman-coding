#pragma once

#include <iostream>
#include <string>

bool hasExtension(const std::string& path);
std::string getExtension(const std::string& path);
std::string stripExtension(const std::string& path);