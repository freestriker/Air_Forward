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
    virtual void OnAdd(T* parent);
    virtual void OnRemove();

    void AddChild(ChildBrotherTree<T>* child);
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

    RTTR_ENABLE()
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
void ChildBrotherTree<T>::OnAdd(T* parent)
{

}
template<class T>
void ChildBrotherTree<T>::OnRemove()
{

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
        child->OnAdd(this->object);
    }
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
    brother->OnAdd(this->parent ? this->parent->object : nullptr);
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