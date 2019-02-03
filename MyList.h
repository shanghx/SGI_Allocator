#pragma once
#include"Alloc.h"
template<class T>
struct ListNode
{
	T _data;
	ListNode<T>* _next;
	ListNode<T>* _prev;

	ListNode(const T& x = 0)
		:_data(x)
		, _next(NULL)
		, _prev(NULL)
	{}
};


template<class T, class Ref, class Ptr>
struct __ListIterator
{
	typedef __ListIterator<T, Ref, Ptr> Self;
	typedef ListNode<T> Node;
	Node* _node;

	__ListIterator()
	{}
	__ListIterator(Node* node)
		:_node(node)
	{}


	Ref operator*() const
	{
		return _node->_data;
	}

	Ptr operator->() const
	{
		return &(_node->_data);
	}
	Self& operator++()
	{
		_node = _node->_next;
		return *this;
	}


	Self& operator++(int)
	{
		Self tmp(*this);
		_node = _node->_next;
		return tmp;
	}
	Self& operator--()
	{
		_node = _node->_prev;
		return *this;
	}



	Self& operator--(int)
	{
		Self tmp(*this);
		_node = _node->_prev;
		return tmp;
	}
	bool operator!=(const Self& It) const
	{
		if (_node != It._node)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool operator==(const Self& It) const
	{
		return It._node == _node;
	}
};
template<class T, class Alloc=alloc>
class MyList
{
	typedef ListNode<T> Node;

public:
	typedef __ListIterator<T, T&, T*> Iterator;
	typedef __ListIterator<T, const T&, const T*> ConstIterator;
	typedef SimpleAlloc<Node, Alloc> ListNodeAllocator;
	Node* BuyNode(const T& x)
	{
		Node* node = ListNodeAllocator::Allocate();
		//显示调用构造函数
		new(node)Node(x);//初始化
		return node;
	}

	void  DestoryNode(Node* node)
	{
		node->~Node();
		ListNodeAllocator::Deallocate(node);
	}
	MyList()
		:_head(BuyNode(T()))
	{
		_head->_next = _head;
		_head->_prev = _head;
	}
	~MyList()
	{
		Clear();
		//delete _head;
		DestoryNode(_head);
		_head = NULL;
	}

	void Clear()
	{

		Iterator cur = Begin();
		while (cur != End())
		{
			Iterator del = cur;
			++cur;
			DestoryNode(del._node);
		}

	

		_head->_next = _head;
		_head->_prev = _head;
	}
	void PushBack(const T& x)
	{
		Insert(End(), x);

	}
	void PopBack()
	{
		Erase(--End());

	}
	void PushFront(const T& x)
	{
		Insert(Begin(), x);
	
	}
	void PopFront()
	{
		Erase(Begin());
	}

	ConstIterator Begin() const
	{
		return ConstIterator(_head->_next);
	}
	ConstIterator End() const
	{
		return ConstIterator(_head);
	}
	Iterator Begin()
	{
		return _head->_next;
	}
	Iterator End()
	{
		return _head;
	}
public:
	
	Node* GetNode(const T& x)
	{
		Node* tmp = ListNodeMalloc::Allocate();
		new(tmp)Node(x);
		return tmp;
	}
	Iterator Insert(Iterator pos, const T& x)
	{
		Node* cur = pos._node;
		Node* prev = cur->_prev;
		//Node* newnode=new
		Node* newnode = BuyNode(x);


		prev->_next = newnode;
		newnode->_prev = prev;
		
		newnode->_next = cur;
		cur->_prev = newnode;

		return newnode;
	}

	Iterator Erase(Iterator&   pos)
	{
		assert(pos != Iterator(_head));
		Node* cur = pos._node;
		Node* prev = cur->_prev;
		Node* next = cur->_next;

		prev->_next = next;
		next->_prev = prev;
		//delete cur;
		DestoryNode(cur);
		pos = prev;
		return next;
		
	}
private:
	Node* _head;
};

void PrintfList(const MyList<int>& l)
{
	MyList<int>::Iterator it = l.Begin()._node;
	while ( it != l.End()._node)
	{
		cout << " it:" << it._node->_data << " ";
		 it++;
	}
	cout << endl;
}
void testList()
{
	MyList<int> L;
	L.PushBack(1);
	L.PushBack(2);
	L.PushBack(3);
	L.PushBack(4);
	L.PushBack(5);
	PrintfList(L);
}
