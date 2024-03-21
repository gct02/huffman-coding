#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include "scoped-handler.hpp"
#include "huffman-encoder.hpp"

class Compressor 
{
private:
	// Alias declarations
	using string = std::string;
	using fstream = std::fstream;
	using ios = std::ios;

public:
	static void zip(const string& inFilePath, const string& outFilePath) 
	{
		RAIIFileHandler scopedInFile(inFilePath, ios::binary | ios::in);
		fstream& inFile = scopedInFile.get();

		RAIIFileHandler scopedOutFile(outFilePath, ios::binary | ios::out);
		fstream& outFile = scopedOutFile.get();

		HuffEncoder encoder(inFile);

		HuffTree huffTree = encoder.getHuffTree();
		HuffDict huffDict = encoder.getHuffDict();

		writeMetadata(outFile, huffTree);

		compress(inFile, outFile, huffDict);
	}

private:
	static constexpr size_t BUFFER_SIZE = 1024;

	Compressor() 
	{
	}

	static uint64_t getHighestFreq(const std::vector<HuffTreeNodePtr>& leaves) 
	{
		return leaves.front()->getFrequency();
	}

	// Returns the minimum number of bytes required to represent a 
	// given unsigned integer.
	static size_t getByteSize(uint64_t num)
	{
		return static_cast<size_t>(ceil(log2(num) / 8));
	}

	// Write the metadata on the beginning of the compressed file.
	// It contains information required to decompress the original file content.
	static void writeMetadata(fstream& outputFile, const HuffTree& huffTree)
	{
		std::vector<HuffTreeNodePtr> leaves = huffTree.getLeaves();

		// Number of distinct bytes in the input file 
		Byte distinctBytes = static_cast<Byte>(leaves.size());

		// Number of bytes required to represent the highest frequency value
		uint64_t highestFreq = getHighestFreq(leaves);
		Byte minBytesFreq = static_cast<Byte>(getByteSize(highestFreq));
		
		size_t metadataSize = (size_t)distinctBytes * (size_t)(minBytesFreq + 1) + 2;
		Byte* metadata = new Byte[metadataSize];

		metadata[0] = distinctBytes;
		metadata[1] = minBytesFreq;

		// Write each distinct byte along with its frequency
		int baseIdx = 2;
		for (HuffTreeNodePtr leaf : leaves) {
			metadata[baseIdx] = leaf->getByte().value();
			uint64_t freq = leaf->getFrequency();

			for (int i = minBytesFreq; i >= 1; i--) {
				metadata[baseIdx + i] = static_cast<Byte>(freq & 0xFF);
				freq >>= 8;
			}

			baseIdx += minBytesFreq + 1;
		}

		outputFile.write(reinterpret_cast<char*>(metadata), metadataSize);
		delete[] metadata;
	}

	static void shiftRightVlc(HuffVLC& vlc, size_t shrSize) 
	{
		// Ensure byte alignment if the VLC's number of bits 
		// is not multiple of 8
		if ((vlc.numBits % 8 == 0) || 
			((vlc.numBits + shrSize) / 8 > vlc.numBits / 8 && 
			(vlc.numBits + shrSize) % 8 != 0)) {
			vlc.code.emplace_back(0);
		}

		size_t shlSize = 8 - shrSize;
		Byte remainingBits = 0;

		// Perform the right shift on each byte of the VLC
		for (size_t i = 0; i < vlc.code.size(); i++) {
			Byte nextRemainingBits = vlc.code[i] << shlSize;
			vlc.code[i] = (vlc.code[i] >> shrSize) | remainingBits;
			remainingBits = nextRemainingBits;
		}

		vlc.numBits += shrSize;
	}

	static void compressByte(Byte byte, HuffVLC& vlcBuff, const HuffDict& huffDict)
	{
		HuffVLC vlc(huffDict.at(byte));
		size_t remainingBits = vlcBuff.numBits % 8;

		vlcBuff.numBits += vlc.numBits;

		if (remainingBits == 0) {
			vlcBuff.code.insert(vlcBuff.code.end(), vlc.code.begin(), vlc.code.end());
		}
		else {
			shiftRightVlc(vlc, remainingBits);

			vlcBuff.code[vlcBuff.code.size() - 1] |= vlc.code[0];
			
			if (vlc.code.size() > 1) {
				vlcBuff.code.insert(vlcBuff.code.end(), vlc.code.begin() + 1, vlc.code.end()
				);
			}
		}
	}

	static void compressChunk(uint8_t* inBuff, size_t inBuffSize, 
		HuffVLC& vlcBuff, const HuffDict& huffDict)
	{
		for (size_t i = 0; i < inBuffSize; i++) {
			compressByte(inBuff[i], vlcBuff, huffDict);
		}
	}

	static void writeVlcToFile(HuffVLC& vlc, fstream& file) 
	{
		vlc.numBits %= 8;

		if (vlc.numBits == 0) {
			for (Byte byte : vlc.code) {
				file.write(reinterpret_cast<char*>(&byte), 1);
			}
			vlc.code.clear();
		}
		else {
			for (int i = 0; i < vlc.code.size() - 1; i++) {
				file.write(reinterpret_cast<char*>(&vlc.code[i]), 1);
			}
			Byte lastByte = vlc.code.back();
			vlc.code.clear();
			vlc.code.emplace_back(lastByte);
		}
	}

	static void compress(fstream& inFile, fstream& outFile, const HuffDict& huffDict)
	{
		Byte inBuff[BUFFER_SIZE];
		HuffVLC vlcBuff;

		vlcBuff.numBits = 0;

		inFile.read(reinterpret_cast<char*>(inBuff), BUFFER_SIZE);
		std::streamsize bytesRead;

		while ((bytesRead = inFile.gcount()) > 0) {
			// Compress chunk of data into a VLC and write it to the output file
			compressChunk(inBuff, bytesRead, vlcBuff, huffDict);
			writeVlcToFile(vlcBuff, outFile);

			inFile.read(reinterpret_cast<char*>(inBuff), BUFFER_SIZE);
		}

		if (vlcBuff.numBits > 0) {
			// Write remaining bits of the VLC to the output file
			outFile.write(reinterpret_cast<char*>(&vlcBuff.code[0]), 1);
		}
	}
};

class Decompressor 
{
private:
	// Alias declarations
	using string = std::string;
	using fstream = std::fstream;
	using ios = std::ios;
	using ByteFreqTable = std::array<uint64_t, 256>;

public:
	static void unzip(const string& inFilePath, const string& outFilePath) 
	{
		RAIIFileHandler scopedInFile(inFilePath, ios::binary | ios::in);
		fstream& inFile = scopedInFile.get();

		RAIIFileHandler scopedOutFile(outFilePath, ios::binary | ios::out);
		fstream& outFile = scopedOutFile.get();

		ByteFreqTable byteFreqs = countFrequencies(inFile);

		HuffEncoder encoder(byteFreqs);
		HuffTree huffTree = encoder.getHuffTree();

		decompress(inFile, outFile, huffTree);
	}

private:
	static constexpr size_t BUFFER_SIZE = 1024;

	Decompressor() 
	{
	}

	static uint8_t* readMetadata(fstream& inFile) {
		uint8_t distinctBytes, minBytesFreq;

		inFile.read(reinterpret_cast<char*>(&distinctBytes), 1);
		inFile.read(reinterpret_cast<char*>(&minBytesFreq), 1);

		size_t metadataSize = (size_t)distinctBytes * ((size_t)minBytesFreq + 1) + 2;
		uint8_t* metadata = new uint8_t[metadataSize];

		metadata[0] = distinctBytes;
		metadata[1] = minBytesFreq;

		inFile.read(reinterpret_cast<char*>(metadata + 2), metadataSize - 2);

		return metadata;
	}

	static ByteFreqTable countFrequencies(fstream& inFile)
	{
		uint8_t* metadata = readMetadata(inFile);

		size_t distinctBytes = static_cast<size_t>(metadata[0]);
		size_t minBytesFreq = static_cast<size_t>(metadata[1]);

		ByteFreqTable byteFreqs = { 0 };

		for (size_t i = 0; i < distinctBytes; i++) {
			size_t idx = i * (minBytesFreq + 1) + 2;

			// Extract the byte along with its frequency from the metadata
			Byte byte = metadata[idx];
			size_t freq = static_cast<size_t>(metadata[idx + 1]);

			// Combine bytes to form the frequency value
			for (int j = 1; j < minBytesFreq; j++) {
				freq <<= 8;
				freq |= static_cast<size_t>(metadata[idx + j + 1]);
			}

			byteFreqs[byte] = freq;
		}
		delete[] metadata;

		return byteFreqs;
	}

	static void decompress(fstream& inFile, fstream& outFile, const HuffTree& huffTree) 
	{
		HuffTreeNodePtr nodePtr = huffTree.getRoot();
		size_t bytesToDecode = nodePtr->getFrequency();
		Byte inByte;

		while (bytesToDecode > 0) {
			Byte currentBit = 0x80;
			inFile.read(reinterpret_cast<char*>(&inByte), 1);

			do {
				// Traverse tree based on the bit value
				if ((inByte & currentBit) == 0) {
					nodePtr = nodePtr->getLeft();
				}
				else {
					nodePtr = nodePtr->getRight();
				}

				if (nodePtr->isLeaf()) {
					Byte decodedByte = nodePtr->getByte().value();
					outFile.write(reinterpret_cast<char*>(&decodedByte), 1);
					bytesToDecode--;

					if (bytesToDecode == 0) {
						break;
					}

					nodePtr = huffTree.getRoot();
				}

				currentBit >>= 1;
			} 
			while (currentBit != 0);
		}
	}
};