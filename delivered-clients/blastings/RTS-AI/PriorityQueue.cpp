#include "PriorityQueue.h"


PriorityQueue::PriorityQueue(int maxNodes) {
	_numNodes = 0;
	_nodes = gcnew array<PriorityQueueNode^>(maxNodes + 1);
	_numNodesEnqueuedTotal = 0;
}

int PriorityQueue::count() {
	return _numNodes;
}

int PriorityQueue::maxSize() {
	return _nodes->Length - 1;
}

void PriorityQueue::enqueue(PriorityQueueNode ^node, double priority) {
	node->priority = priority;
	_numNodes++;
	_nodes[_numNodes] = node;
	node->queueIndex = _numNodes;
	node->insertIndex = _numNodesEnqueuedTotal++;
	cascadeUp(_nodes[_numNodes]);
}

PriorityQueueNode^ PriorityQueue::dequeue() {
	PriorityQueueNode ^node = _nodes[1];
	remove(node);
	return node;
}

void PriorityQueue::remove(PriorityQueueNode ^node) {
	if (!contains(node)) {
		return;
	}

	if (_numNodes <= 1) {
		_nodes[1] = nullptr;
		_numNodes = 0;
		return;
	}

	//Make sure the node is the last node in the queue
	bool wasSwapped = false;
	PriorityQueueNode ^formerLastNode = _nodes[_numNodes];
	if (node->queueIndex != _numNodes) {
		swap(node, formerLastNode);
		wasSwapped = true;
	}

	_numNodes--;
	_nodes[node->queueIndex] = nullptr;

	if (wasSwapped) {
		onNodeUpdated(formerLastNode);
	}
}

void PriorityQueue::updatePriority(PriorityQueueNode ^node, double priority) {
	node->priority = priority;
	onNodeUpdated(node);
}

bool PriorityQueue::isValidQueue() {
	 for (int i = 1; i < _nodes->Length; i++) {
		 if (_nodes[i]) {
			 int childLeftIndex = 2 * i;
			 if (childLeftIndex < _nodes->Length && _nodes[childLeftIndex] && hasHigherPriority(_nodes[childLeftIndex], _nodes[i]))
				 return false;

			 int childRightIndex = childLeftIndex + 1;
			 if (childRightIndex < _nodes->Length && _nodes[childRightIndex] && hasHigherPriority(_nodes[childRightIndex], _nodes[i]))
				 return false;
		 }
	 }
	 return true;
 }

bool PriorityQueue::contains(PriorityQueueNode ^node) {
	return (_nodes[node->queueIndex] == node);
}

void PriorityQueue::swap(PriorityQueueNode ^node1, PriorityQueueNode ^node2) {
	_nodes[node1->queueIndex] = node2;
	_nodes[node2->queueIndex] = node1;

	//Swap their indices
	int temp = node1->queueIndex;
	node1->queueIndex = node2->queueIndex;
	node2->queueIndex = temp;

}

void PriorityQueue::cascadeUp(PriorityQueueNode ^node) {
	int parent = node->queueIndex / 2;
	while (parent >= 1) {
		PriorityQueueNode ^parentNode = _nodes[parent];
		if (hasHigherPriority(parentNode, node)) {
			break;
		}

		//Node has lower priority value, so move it up the heap
		swap(node, parentNode);

		parent = node->queueIndex / 2;
	}
}

void PriorityQueue::cascadeDown(PriorityQueueNode ^node) {
	 PriorityQueueNode ^newParent = nullptr;
	 int finalQueueIndex = node->queueIndex;
	 while (true) {
		 newParent = node;
		 int childLeftIndex = 2 * finalQueueIndex;

		 //Check if the left child has higher priority than the current node
		 if (childLeftIndex > _numNodes) {
			 node->queueIndex = finalQueueIndex;
			 _nodes[finalQueueIndex] = node;
			 break;
		 }

		 PriorityQueueNode ^childLeft = _nodes[childLeftIndex];
		 if (hasHigherPriority(childLeft, newParent)) {
			 newParent = childLeft;
		 }

		 //Check if the right child has higher priority than the current node or the left child
		 int childRightIndex = childLeftIndex + 1;
		 if (childRightIndex <= _numNodes) {
			 PriorityQueueNode ^childRight = _nodes[childRightIndex];
			 if (hasHigherPriority(childRight, newParent)) {
				 newParent = childRight;
			 }
		 }

		 //If either of the children has higher priority, swap and continue cascading
		 if (newParent != node) {
			 _nodes[finalQueueIndex] = newParent;

			 int temp = newParent->queueIndex;
			 newParent->queueIndex = finalQueueIndex;
			 finalQueueIndex = temp;
		 } else {
			 //See note above
			 node->queueIndex = finalQueueIndex;
			 _nodes[finalQueueIndex] = node;
			 break;
		 }
	 }
 }

bool PriorityQueue::hasHigherPriority(PriorityQueueNode ^higher, PriorityQueueNode ^lower) {
	return (higher->priority < lower->priority || (higher->priority == lower->priority && higher->insertIndex < lower->insertIndex));
}

void PriorityQueue::onNodeUpdated(PriorityQueueNode ^node) {
	int parentIndex = node->queueIndex / 2;
	PriorityQueueNode ^parentNode = _nodes[parentIndex];

	if (parentIndex > 0 && hasHigherPriority(node, parentNode)) {
		cascadeUp(node);
	} else {
		cascadeDown(node);
	}
}

