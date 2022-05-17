#pragma once

namespace Utils
{
	class CrossLinkableNode
	{
		friend class CrossLinkableColHead;
		friend class CrossLinkableRowHead;
	private:
		CrossLinkableNode* left;
		CrossLinkableNode* right;
		CrossLinkableNode* top;
		CrossLinkableNode* bottom;
		CrossLinkableNode* head;
	protected:
		CrossLinkableNode();
		virtual ~CrossLinkableNode();
	};
	class CrossLinkableColHead final
	{
	private:
		CrossLinkableNode head;
		CrossLinkableNode* end;
	public:
		class Itertor final
		{
			friend class CrossLinkableColHead;
		private:
			CrossLinkableNode* node;
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
		CrossLinkableNode head;
		CrossLinkableNode* end;
	public:
		class Itertor final
		{
			friend class CrossLinkableRowHead;
		private:
			CrossLinkableNode* node;
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
