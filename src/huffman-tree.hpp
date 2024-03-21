#pragma once

#include <vector>
#include <memory>
#include <array>
#include <optional>
#include <stdexcept>

class HuffTreeNode;
using HuffTreeNodePtr = std::shared_ptr<HuffTreeNode>;

class HuffTreeNode 
{
public:
	HuffTreeNode(uint64_t frequency = 0)
		: frequency(frequency) 
	{
	}

	HuffTreeNode(uint8_t byte, uint64_t frequency = 0)
		: byte(byte), frequency(frequency) 
	{
	}

	HuffTreeNode(HuffTreeNodePtr left, HuffTreeNodePtr right)
		: left(left), right(right) 
	{
		if (this->left.get() == nullptr || this->right.get() == nullptr) {
			throw std::invalid_argument("Internal node shouldn't have a null child.");
		}
		
		frequency = left->frequency + right->frequency;
	}

	std::optional<uint8_t> getByte() const 
	{
		return byte;
	}

	uint64_t getFrequency() const 
	{
		return frequency;
	}

	HuffTreeNodePtr getLeft() const
	{
		return left;
	}

	HuffTreeNodePtr getRight() const
	{
		return right;
	}

	void setByte(uint8_t byte) 
	{
		if (left != nullptr || right != nullptr) {
			throw std::logic_error("Internal node's byte shouldn't be settled.");
		}

		this->byte = byte;
	}

	void setFrequency(uint64_t frequency) 
	{
		this->frequency = frequency;
	}

	void setSubtrees(HuffTreeNodePtr left, HuffTreeNodePtr right)
	{
		if (byte.has_value()) {
			throw std::logic_error("Nodes with settled byte attribute should be leaves.");
		}

		if (left == nullptr || right == nullptr) {
			throw std::invalid_argument("Internal node shouldn't have a null child.");
		}

		this->left = left;
		this->right = right;
		frequency = left->getFrequency() + right->getFrequency();
	}

	bool isLeaf() const 
	{
		return byte.has_value();
	}

private:
	std::optional<uint8_t> byte;
	uint64_t frequency;
	HuffTreeNodePtr left, right;
};

class HuffTree {
private:
	// Alias declarations
	using ByteFreqTable = std::array<uint64_t, 256>;

public:
	HuffTree() 
	{
	}

	HuffTree(const ByteFreqTable& byteFreqs)
	{
		for (uint16_t i = 0; i < 256; i++) {
			if (byteFreqs[i] > 0) {
				HuffTreeNode node((uint8_t)i, byteFreqs[i]);
				HuffTreeNodePtr nodePtr = std::make_shared<HuffTreeNode>(node);

				std::vector<HuffTreeNodePtr>::iterator iter = leaves.begin();
				std::vector<HuffTreeNodePtr>::iterator end = leaves.end();

				while (iter < end && (*iter)->getFrequency() > nodePtr->getFrequency()) {
					iter++;
				}

				leaves.insert(iter, nodePtr);
			}
		}

		buildTree();
	}

	HuffTreeNodePtr getRoot() const 
	{
		return root;
	}

	std::vector<HuffTreeNodePtr> getLeaves() const 
	{
		return leaves;
	}

private:
	HuffTreeNodePtr root;
	std::vector<HuffTreeNodePtr> leaves;

	void buildTree()
	{
		// Temporary priority queue to build tree from its leaves
		std::vector<HuffTreeNodePtr> tempPriorityQueue(leaves);

		while (tempPriorityQueue.size() > 1) {
			HuffTreeNodePtr left = tempPriorityQueue.back();
			tempPriorityQueue.pop_back();

			HuffTreeNodePtr right = tempPriorityQueue.back();
			tempPriorityQueue.pop_back();

			HuffTreeNode node(left, right);
			HuffTreeNodePtr nodePtr = std::make_shared<HuffTreeNode>(node);

			std::vector<HuffTreeNodePtr>::iterator iter = tempPriorityQueue.begin();
			std::vector<HuffTreeNodePtr>::iterator end = tempPriorityQueue.end();

			while (iter < end && (*iter)->getFrequency() > nodePtr->getFrequency()) {
				iter++;
			}

			tempPriorityQueue.insert(iter, nodePtr);
		}

		// The only remaining node is the root of the tree
		root = tempPriorityQueue.front();
	}
};