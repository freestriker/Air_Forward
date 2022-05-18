#pragma once

namespace Utils
{
	class CrossLinkableNode
	{
		friend class CrossLinkableColHead;
		friend class CrossLinkableRowHead;
	private:
		CrossLinkableNode* _left;
		CrossLinkableNode* _right;
		CrossLinkableNode* _top;
		CrossLinkableNode* _bottom;
		CrossLinkableColHead* _colHead;
		CrossLinkableRowHead* _rowHead;
	protected:
		CrossLinkableNode();
		virtual ~CrossLinkableNode();
		CrossLinkableColHead* ColHead();
		CrossLinkableRowHead* RowHead();
		void RemoveSelf();
	};
	class CrossLinkableColHead final
	{
	private:
		CrossLinkableNode _head;
		CrossLinkableNode* _end;
	public:
		class Itertor final
		{
			friend class CrossLinkableColHead;
		private:
			CrossLinkableNode* _node;
			Itertor(CrossLinkableNode* node);
		public:
			~Itertor();
			bool IsValid();
			Itertor operator++();
			Itertor operator++(int);
			Itertor operator--();
			Itertor operator--(int);
			CrossLinkableNode* Node();
		};
		CrossLinkableColHead();
		virtual ~CrossLinkableColHead();

		void Add(CrossLinkableNode* node);
		void Remove(CrossLinkableNode* node);
		Itertor GetItertor();
	};
	class CrossLinkableRowHead final
	{
	private:
		CrossLinkableNode _head;
		CrossLinkableNode* _end;
	public:
		class Itertor final
		{
			friend class CrossLinkableRowHead;
		private:
			CrossLinkableNode* _node;
			Itertor(CrossLinkableNode* node);
		public:
			~Itertor();
			bool IsValid();
			Itertor operator++();
			Itertor operator++(int);
			Itertor operator--();
			Itertor operator--(int);
			CrossLinkableNode* Node();
		};
		CrossLinkableRowHead();
		virtual ~CrossLinkableRowHead();

		void Add(CrossLinkableNode* node);
		void Remove(CrossLinkableNode* node);
		Itertor GetItertor();
	};
}
