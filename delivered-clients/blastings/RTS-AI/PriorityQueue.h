#pragma once

#include "PriorityQueueNode.h"

using namespace System::Collections::Generic;

public ref class PriorityQueue {
private:
	int _numNodes;
	array<PriorityQueueNode^> ^_nodes;
	int _numNodesEnqueuedTotal;

public:
	PriorityQueue(int maxNode);

	int count();
	int maxSize();
	
	void enqueue(PriorityQueueNode ^node, double priority);
	PriorityQueueNode^ dequeue();
	void remove(PriorityQueueNode ^node);
	void updatePriority(PriorityQueueNode ^node, double priority);
	bool isValidQueue();
	bool contains(PriorityQueueNode ^node);

protected:
	void cascadeUp(PriorityQueueNode ^node);
	void cascadeDown(PriorityQueueNode ^node);
	bool hasHigherPriority(PriorityQueueNode ^higher, PriorityQueueNode ^lower);
	void onNodeUpdated(PriorityQueueNode ^node);
	void swap(PriorityQueueNode ^node1, PriorityQueueNode ^node2);
			
};