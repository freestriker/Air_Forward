#include "Utils/CrossLinkableNode.h"
#include "Utils/Log.h"

Utils::CrossLinkableNode::CrossLinkableNode()
	: _left(nullptr)
	, _right(nullptr)
	, _top(nullptr)
	, _bottom(nullptr)
	, _colHead(nullptr)
	, _rowHead(nullptr)
{
}

Utils::CrossLinkableNode::~CrossLinkableNode()
{
}

Utils::CrossLinkableColHead* Utils::CrossLinkableNode::ColHead()
{
	return _colHead;
}

Utils::CrossLinkableRowHead* Utils::CrossLinkableNode::RowHead()
{
	return _rowHead;
}

void Utils::CrossLinkableNode::RemoveSelf()
{
	_colHead->Remove(this);
	_rowHead->Remove(this);
}

Utils::CrossLinkableColHead::CrossLinkableColHead()
	: _head()
	, _end(nullptr)
{
	_head._colHead = this;
	_end = &_head;
}

Utils::CrossLinkableColHead::~CrossLinkableColHead()
{
}

void Utils::CrossLinkableColHead::Add(CrossLinkableNode* node)
{
	_end->_bottom = node;
	node->_top = _end;
	node->_bottom = nullptr;

	_end = node;

	node->_colHead = this;
}

void Utils::CrossLinkableColHead::Remove(CrossLinkableNode* node)
{
	if (node->_colHead != this)
	{
		Utils::Log::Exception("The cross linked node do not blong to this head.");
	}

	if (node != _end)
	{
		node->_top->_bottom = node->_bottom;
		node->_bottom->_top = node->_top;
	}
	else
	{
		node->_top->_bottom = nullptr;

		_end = node->_top;
	}

	node->_top = nullptr;
	node->_bottom = nullptr;
	node->_colHead = nullptr;
}

Utils::CrossLinkableColHead::Iterator Utils::CrossLinkableColHead::Remove(Iterator iterator)
{
	if (!iterator.IsValid())
	{
		Utils::Log::Exception("This Iterator is not valid.");
	}
	auto node = iterator._node;
	Iterator next = Iterator();
	if (node->_colHead != this)
	{
		Utils::Log::Exception("The cross linked node do not blong to this head.");
	}

	if (node != _end)
	{
		next._node = node->_bottom;

		node->_top->_bottom = node->_bottom;
		node->_bottom->_top = node->_top;
	}
	else
	{
		next._node = &node->_colHead->_head;

		node->_top->_bottom = nullptr;

		_end = node->_top;
	}

	node->_top = nullptr;
	node->_bottom = nullptr;
	node->_colHead = nullptr;
	return next;
}

bool Utils::CrossLinkableColHead::HaveNode()
{
	return _end != &_head;
}

Utils::CrossLinkableColHead::Iterator Utils::CrossLinkableColHead::GetIterator()
{
	return Iterator(_head._bottom);
}

Utils::CrossLinkableColHead::Iterator::Iterator()
	: Iterator(nullptr)
{
}

Utils::CrossLinkableColHead::Iterator::Iterator(CrossLinkableNode* node)
	: _node(node)
{
}

Utils::CrossLinkableColHead::Iterator::~Iterator()
{
}

bool Utils::CrossLinkableColHead::Iterator::IsValid()
{
	return _node && _node != &_node->_colHead->_head;
}

Utils::CrossLinkableColHead::Iterator Utils::CrossLinkableColHead::Iterator::operator++()
{
	_node = _node->_bottom;
	return *this;
}

Utils::CrossLinkableColHead::Iterator Utils::CrossLinkableColHead::Iterator::operator++(int)
{
	Utils::CrossLinkableColHead::Iterator t = *this;
	_node = _node->_bottom;
	return t;
}

Utils::CrossLinkableColHead::Iterator Utils::CrossLinkableColHead::Iterator::operator--()
{
	_node = _node->_top;
	return *this;
}

Utils::CrossLinkableColHead::Iterator Utils::CrossLinkableColHead::Iterator::operator--(int)
{
	Utils::CrossLinkableColHead::Iterator t = *this;
	_node = _node->_top;
	return t;
}

Utils::CrossLinkableNode* Utils::CrossLinkableColHead::Iterator::Node()
{
	return _node;
}

Utils::CrossLinkableRowHead::CrossLinkableRowHead()
	: _head()
	, _end(nullptr)
{
	_head._rowHead = this;
	_end = &_head;
}

Utils::CrossLinkableRowHead::~CrossLinkableRowHead()
{
}

bool Utils::CrossLinkableRowHead::HaveNode()
{
	return _end != &_head;
}

void Utils::CrossLinkableRowHead::Add(CrossLinkableNode* node)
{
	_end->_right = node;
	node->_left = _end;
	node->_right = nullptr;

	_end = node;

	node->_rowHead = this;
}

void Utils::CrossLinkableRowHead::Remove(CrossLinkableNode* node)
{
	if (node->_rowHead != this)
	{
		Utils::Log::Exception("The cross linked node do not blong to this head.");
	}

	if (node != _end)
	{
		node->_left->_right = node->_right;
		node->_right->_left = node->_left;
	}
	else
	{
		node->_left->_right = nullptr;

		_end = node->_left;
	}

	node->_left = nullptr;
	node->_right = nullptr;
	node->_rowHead = nullptr;
}

Utils::CrossLinkableRowHead::Iterator Utils::CrossLinkableRowHead::Remove(Iterator iterator)
{
	if (!iterator.IsValid())
	{
		Utils::Log::Exception("This Iterator is not valid.");
	}
	auto node = iterator._node;
	Iterator next = Iterator();
	if (node->_rowHead != this)
	{
		Utils::Log::Exception("The cross linked node do not blong to this head.");
	}

	if (node != _end)
	{
		next._node = node->_right;

		node->_left->_right = node->_right;
		node->_right->_left = node->_left;
	}
	else
	{
		next._node = &node->_rowHead->_head;

		node->_left->_right = nullptr;

		_end = node->_left;
	}

	node->_left = nullptr;
	node->_right = nullptr;
	node->_rowHead = nullptr;
	return next;
}

Utils::CrossLinkableRowHead::Iterator Utils::CrossLinkableRowHead::GetIterator()
{
	return Iterator(_head._right);
}

Utils::CrossLinkableRowHead::Iterator::Iterator()
	: Iterator(nullptr)
{
}

Utils::CrossLinkableRowHead::Iterator::Iterator(CrossLinkableNode* node)
	: _node(node)
{
}

Utils::CrossLinkableRowHead::Iterator::~Iterator()
{
}

bool Utils::CrossLinkableRowHead::Iterator::IsValid()
{
	return _node && _node != &_node->_rowHead->_head;
}

Utils::CrossLinkableRowHead::Iterator Utils::CrossLinkableRowHead::Iterator::operator++()
{
	_node = _node->_right;
	return *this;
}

Utils::CrossLinkableRowHead::Iterator Utils::CrossLinkableRowHead::Iterator::operator++(int)
{
	Utils::CrossLinkableRowHead::Iterator t = *this;
	_node = _node->_right;
	return t;
}

Utils::CrossLinkableRowHead::Iterator Utils::CrossLinkableRowHead::Iterator::operator--()
{
	_node = _node->_left;
	return *this;
}

Utils::CrossLinkableRowHead::Iterator Utils::CrossLinkableRowHead::Iterator::operator--(int)
{
	Utils::CrossLinkableRowHead::Iterator t = *this;
	_node = _node->_left;
	return t;
}

Utils::CrossLinkableNode* Utils::CrossLinkableRowHead::Iterator::Node()
{
	return _node;
}
