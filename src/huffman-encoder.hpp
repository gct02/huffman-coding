#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <unordered_map>

#include "huffman-tree.hpp"

using Byte = uint8_t;

// Struct that represents a Huffman Coding variable-lenght code (VLC)
// associated with a determined byte.
struct HuffVLC 
{
	std::vector<Byte> code;
	size_t numBits;
};

// Mapping between bytes and its associated Huffman Coding VLCs
using HuffDict = std::unordered_map<Byte, HuffVLC>;

class HuffEncoder 
{
private:
	// Alias declarations
	using string = std::string;
	using fstream = std::fstream;
	using ByteFreqTable = std::array<uint64_t, 256>;

public:
	HuffEncoder(fstream& inFile) 
	{
		ByteFreqTable byteFreqs = countFrequencies(inFile);

		// Set file back to its initial state
		inFile.clear();
		inFile.seekg(0, std::ios::beg);

		huffTree = HuffTree(byteFreqs);
		buildHuffDict();
	}

	HuffEncoder(ByteFreqTable byteFreqs) 
	{
		huffTree = HuffTree(byteFreqs);
		buildHuffDict();
	}

	HuffDict getHuffDict() const 
	{
		return huffDict;
	}

	HuffTree getHuffTree() const 
	{
		return huffTree;
	}

private:
	HuffTree huffTree;
	HuffDict huffDict;

	// Returns the frequency of each distinct byte in the given file.
	ByteFreqTable countFrequencies(fstream& inFile) 
	{
		ByteFreqTable byteFreqs{ 0 };
		Byte byte;

		while (inFile.read(reinterpret_cast<char*>(&byte), 1)) {
			byteFreqs[byte]++;
		}

		return byteFreqs;
	}

	void setVLC(HuffVLC vlc, HuffTreeNodePtr nodePtr, Byte currentPos) 
	{
		if (nodePtr->isLeaf()) {
			// Leaf node: Assign encoding mapping
			Byte byte = nodePtr->getByte().value();
			huffDict.insert({ byte, vlc });
		}
		else {
			// Internal node: Traverse left and right subtrees
			vlc.numBits++;

			if (currentPos == 0) {
				currentPos = 0x80;
				vlc.code.emplace_back(0);
			}

			setVLC(vlc, nodePtr->getLeft(), currentPos >> 1);
			vlc.code[vlc.code.size() - 1] |= currentPos;
			setVLC(vlc, nodePtr->getRight(), currentPos >> 1);
		}
	}

	void buildHuffDict() 
	{
		Byte initPos = 0x80; // 10000000 in binary
		HuffVLC initVLC = { std::vector<Byte>(1, 0), 0 };
		setVLC(initVLC, huffTree.getRoot(), initPos);
	}
};