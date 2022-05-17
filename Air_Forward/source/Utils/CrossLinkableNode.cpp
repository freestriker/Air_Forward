#include "Utils/CrossLinkableNode.h"
#include "Utils/Log.h"

Utils::CrossLinkableNode::CrossLinkableNode()
	: left(nullptr)
	, right(nullptr)
	, top(nullptr)
	, bottom(nullptr)
	, head(nullptr)
{
}

Utils::CrossLinkableNode::~CrossLinkableNode()
{
}

Utils::CrossLinkableColHead::CrossLinkableColHead()
	: head()
	, end(nullptr)
{
	head.head = &head;
	end = &head;
}

Utils::CrossLinkableColHead::~CrossLinkableColHead()
{
}

void Utils::CrossLinkableColHead::Add(CrossLinkableNode* node)
{
	end->bottom = node;
	node->top = end;
	node->bottom = nullptr;

	end = node;

	node->head = &head;
}

void Utils::CrossLinkableColHead::Remove(CrossLinkableNode* node)
{
	if (node->head == &head)
	{
		if (node->bottom)
		{
			node->top->bottom = node->bottom;
			node->bottom->top = node->top;
		}
		else
		{
			node->top->bottom = nullptr;

			end = node->top;
		}

		node->top = nullptr;
		node->bottom = nullptr;
		node->head = nullptr;
	}
	else
	{
		Utils::Log::Exception("The cross linked node do not blong to this head.");
	}
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::GetItertor()
{
	return Itertor(head.bottom);
}

Utils::CrossLinkableColHead::Itertor::Itertor(CrossLinkableNode* node)
	: node(node)
{
}

Utils::CrossLinkableColHead::Itertor::~Itertor()
{
}

bool Utils::CrossLinkableColHead::Itertor::IsValid()
{
	return node && node != node->head;
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::Itertor::operator++()
{
	node = node->bottom;
	return *this;
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::Itertor::operator++(int)
{
	Utils::CrossLinkableColHead::Itertor t = *this;
	node = node->bottom;
	return t;
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::Itertor::operator--()
{
	node = node->top;
	return *this;
}

Utils::CrossLinkableColHead::Itertor Utils::CrossLinkableColHead::Itertor::operator--(int)
{
	Utils::CrossLinkableColHead::Itertor t = *this;
	node = node->top;
	return t;
}

Utils::CrossLinkableNode* Utils::CrossLinkableColHead::Itertor::Node()
{
	return node;
}

Utils::CrossLinkableRowHead::CrossLinkableRowHead()
	: head()
	, end(nullptr)
{
	head.head = &head;
	end = &head;
}

Utils::CrossLinkableRowHead::~CrossLinkableRowHead()
{
}

void Utils::CrossLinkableRowHead::Add(CrossLinkableNode* node)
{
	end->right = node;
	node->left = end;
	node->right = nullptr;

	end = node;

	node->head = &head;
}

void Utils::CrossLinkableRowHead::Remove(CrossLinkableNode* node)
{
	if (node->head == &head)
	{
		if (node->right)
		{
			node->left->right = node->right;
			node->right->left = node->left;
		}
		else
		{
			node->left->right = nullptr;

			end = node->left;
		}

		node->left = nullptr;
		node->right = nullptr;
		node->head = nullptr;
	}
	else
	{
		Utils::Log::Exception("The cross linked node do not blong to this head.");
	}
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::GetItertor()
{
	return Itertor(head.right);
}

Utils::CrossLinkableRowHead::Itertor::Itertor(CrossLinkableNode* node)
	: node(node)
{
}

Utils::CrossLinkableRowHead::Itertor::~Itertor()
{
}

bool Utils::CrossLinkableRowHead::Itertor::IsValid()
{
	return node && node != node->head;
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::Itertor::operator++()
{
	node = node->right;
	return *this;
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::Itertor::operator++(int)
{
	Utils::CrossLinkableRowHead::Itertor t = *this;
	node = node->right;
	return t;
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::Itertor::operator--()
{
	node = node->left;
	return *this;
}

Utils::CrossLinkableRowHead::Itertor Utils::CrossLinkableRowHead::Itertor::operator--(int)
{
	Utils::CrossLinkableRowHead::Itertor t = *this;
	node = node->left;
	return t;
}

Utils::CrossLinkableNode* Utils::CrossLinkableRowHead::Itertor::Node()
{
	return node;
}
