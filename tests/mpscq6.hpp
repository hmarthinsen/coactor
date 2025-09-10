#pragma once

#include <utility>

namespace coactor {

template <typename T>
struct MpscQueueNode {
	MpscQueueNode* volatile next;
	T data{};
};

template <typename T>
class MpscQueue {
public:
	MpscQueue(MpscQueueNode<T>* stub)
	{
		stub->next = nullptr;
		m_head = stub;
		m_tail = stub;
	}

	void push(MpscQueueNode<T>* n)
	{
		n->next = nullptr;
		// serialization-point wrt producers, acquire-release
		// FIXME: Shoud be atomic exchange
		MpscQueueNode<T>* prev = std::swap(&m_head, n);
		prev->next = n; // serialization-point wrt consumer, release
	}

	MpscQueueNode<T>* pop()
	{
		MpscQueueNode<T>* tail = m_tail;
		// serialization-point wrt producers, acquire
		MpscQueueNode<T>* next = tail->next;
		if (next) {
			m_tail = next;
			tail->state = next->state;
			return tail;
		}

		return nullptr;
	}

private:
	MpscQueueNode<T>* volatile m_head;
	MpscQueueNode<T>* m_tail;
};

} // namespace coactor
