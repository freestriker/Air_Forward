#pragma once
template<class T>
class ChildBrotherTree
{

public:
    struct ChildIterator 
    {
        ChildBrotherTree<T>* _node;

    public:
        ChildIterator(T* node)
        {
            node ? this->_node = node->child : nullptr;

        }
        ~ChildIterator()
        {
            this->_node = nullptr;

        }
        bool IsValid()
        {
            return _node;
        }
        ChildBrotherTree<T>* Node()
        {
            return _node;
        }
        ChildIterator operator++() 
        {
            _node = _node->brother;
            return *this;
        }
    };

    ChildBrotherTree<T>* parent;
    ChildBrotherTree<T>* child;
    ChildBrotherTree<T>* brother;
    T* object;

    ChildBrotherTree(ChildBrotherTree<T>* parent, ChildBrotherTree<T>* child, ChildBrotherTree<T>* brother, T* object);
    ChildBrotherTree(ChildBrotherTree<T>* parent, T* object);
    ChildBrotherTree(T* object);
    ChildBrotherTree();
    ~ChildBrotherTree();
    virtual void OnAdd(ChildBrotherTree<T>* parent);
    virtual void OnRemove();

    ChildBrotherTree<T>* AddChild(T* object);
    void AddChild(ChildBrotherTree<T>* child);
    ChildBrotherTree<T>* AddBrother(T* object);
    void AddBrother(ChildBrotherTree<T>* brother);
    ChildBrotherTree<T>* Remove();
    ChildIterator GetChildIterator()
    {
        return ChildIterator(this);
    }
    ChildIterator GetBrotherIterator()
    {
        return ChildIterator(this->parent);
    }
};
template<class T>
ChildBrotherTree<T>::ChildBrotherTree() :ChildBrotherTree(nullptr, nullptr, nullptr, nullptr)
{

}
template<class T>
ChildBrotherTree<T>::~ChildBrotherTree()
{
    parent = nullptr;
    child = nullptr;
    brother = nullptr;
    object = nullptr;
}
template<class T>
ChildBrotherTree<T>::ChildBrotherTree(ChildBrotherTree* parent, ChildBrotherTree* child, ChildBrotherTree* brother, T* object)
{
    this->parent = parent;
    this->child = child;
    this->brother = brother;
    this->object = object;
}
template<class T>
ChildBrotherTree<T>::ChildBrotherTree(ChildBrotherTree* parent, T* object) :ChildBrotherTree(parent, nullptr, nullptr, object)
{

}
template<class T>
ChildBrotherTree<T>::ChildBrotherTree(T* object) :ChildBrotherTree(nullptr, nullptr, nullptr, object)
{

}

template<class T>
void ChildBrotherTree<T>::OnAdd(ChildBrotherTree<T>* parent)
{

}
template<class T>
void ChildBrotherTree<T>::OnRemove()
{

}


template<class T>
ChildBrotherTree<T>* ChildBrotherTree<T>::AddChild(T* object)
{
    ChildBrotherTree<T>* newChild = new ChildBrotherTree<T>(object);
    this->AddChild(newChild);
    return newChild;
}
template<class T>
void ChildBrotherTree<T>::AddChild(ChildBrotherTree<T>* child)
{
    child->parent = this;
    if (this->child)
    {
        this->child->AddBrother(child);
    }
    else
    {
        this->child = child;
        child->OnAdd(this);
    }
}

template<class T>
ChildBrotherTree<T>* ChildBrotherTree<T>::AddBrother(T* object)
{
    ChildBrotherTree<T>* newBrother = new ChildBrotherTree<T>(object);
    AddBrother(newBrother);
    return newBrother;
}
template<class T>
void ChildBrotherTree<T>::AddBrother(ChildBrotherTree<T>* brother)
{
    brother->parent = this->parent;
    ChildBrotherTree<T>* b = this->brother;
    if (b)
    {
        while (b->brother)
        {
            b = b->brother;
        }
        b->brother = brother;
    }
    else
    {
        this->brother = brother;
    }
    brother->OnAdd(this->parent);
}

template<class T>
ChildBrotherTree<T>* ChildBrotherTree<T>::Remove()
{
    ChildBrotherTree<T>* result = nullptr;
    if (this->parent)
    {
        T* pre = nullptr;
        ChildBrotherTree<T>* o = this->parent;
        for (ChildIterator start = o->GetStartChildIterator(), end = o->GetEndChildIterator(); start != end; ++start)
        {
            if ((*start) == this)
            {
                break;
            }
            pre = *start;
        }
        if (pre)
        {
            pre->brother = this->brother;
            this->parent = nullptr;
            this->brother = nullptr;
        }
        else
        {
            this->parent->child = this->brother;
        }
        this->parent = nullptr;
        this->brother = nullptr;
        result = this;
    }
    else
    {
        this->parent = nullptr;
        this->brother = nullptr;
        result = this;
    }
    result->OnRemove();
    return result;
}