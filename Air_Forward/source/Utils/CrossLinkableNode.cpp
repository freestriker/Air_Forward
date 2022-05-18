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

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::GetItertor()
{
	return Itertor(_head._bottom);
}

Utils::CrossLinkableColHead::Itertor::Itertor(CrossLinkableNode* node)
	: _node(node)
{
}

Utils::CrossLinkableColHead::Itertor::~Itertor()
{
}

bool Utils::CrossLinkableColHead::Itertor::IsValid()
{
	return _node && _node != &_node->_colHead->_head;
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::Itertor::operator++()
{
	_node = _node->_bottom;
	return *this;
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::Itertor::operator++(int)
{
	Utils::CrossLinkableColHead::Itertor t = *this;
	_node = _node->_bottom;
	return t;
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::Itertor::operator--()
{
	_node = _node->_top;
	return *this;
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::Itertor::operator--(int)
{
	Utils::CrossLinkableColHead::Itertor t = *this;
	_node = _node->_top;
	return t;
}

Utils::CrossLinkableNode* Utils::CrossLinkableColHead::Itertor::Node()
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

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::GetItertor()
{
	return Itertor(_head._right);
}

Utils::CrossLinkableRowHead::Itertor::Itertor(CrossLinkableNode* node)
	: _node(node)
{
}

Utils::CrossLinkableRowHead::Itertor::~Itertor()
{
}

bool Utils::CrossLinkableRowHead::Itertor::IsValid()
{
	return _node && _node != &_node->_rowHead->_head;
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::Itertor::operator++()
{
	_node = _node->_right;
	return *this;
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::Itertor::operator++(int)
{
	Utils::CrossLinkableRowHead::Itertor t = *this;
	_node = _node->_right;
	return t;
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::Itertor::operator--()
{
	_node = _node->_left;
	return *this;
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::Itertor::operator--(int)
{
	Utils::CrossLinkableRowHead::Itertor t = *this;
	_node = _node->_left;
	return t;
}

Utils::CrossLinkableNode* Utils::CrossLinkableRowHead::Itertor::Node()
{
	return _node;
}
