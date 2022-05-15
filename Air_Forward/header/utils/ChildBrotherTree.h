#pragma once
namespace Utils
{
    template<typename T>
    class ChildBrotherTree
    {
    public:
        struct Iterator
        {
            friend class ChildBrotherTree<T>;
        private:
            ChildBrotherTree<T>* _node;
            Iterator(ChildBrotherTree<T>* node);
        public:
            Iterator();
            ~Iterator();
            inline bool IsValid();
            inline typename ChildBrotherTree<T>& Node();
            inline typename Iterator operator++();
        };
    private:
        ChildBrotherTree<T>* _parent;
        ChildBrotherTree<T>* _child;
        ChildBrotherTree<T>* _brother;
        T* _object;
    public:
        ChildBrotherTree(ChildBrotherTree<T>* parent, ChildBrotherTree<T>* child, ChildBrotherTree<T>* brother, T* object);
        ChildBrotherTree(ChildBrotherTree<T>* parent, T* object);
        ChildBrotherTree(T* object);
        ChildBrotherTree();
        ~ChildBrotherTree();

        inline void SetObject(T* object);

        inline bool IsParentValid();
        inline bool IsChildValid();
        inline bool IsBrotherValid();
        inline ChildBrotherTree<T>& Parent();
        inline ChildBrotherTree<T>& Child();
        inline ChildBrotherTree<T>& Brother();
        inline T& Object();

        inline void AddChild(ChildBrotherTree<T>& child);
        inline void AddBrother(ChildBrotherTree<T>& brother);
        inline ChildBrotherTree<T>& Remove();
        inline ChildBrotherTree<T>::Iterator GetChildIterator();
        inline ChildBrotherTree<T>::Iterator GetBrotherIterator();
    };


    template<typename T>
    ChildBrotherTree<T>::ChildBrotherTree(ChildBrotherTree* parent, ChildBrotherTree* child, ChildBrotherTree* brother, T* object)
        : _parent(parent)
        , _child(child)
        , _brother(brother)
        , _object(object)
    {

    }

    template<typename T>
    ChildBrotherTree<T>::ChildBrotherTree(ChildBrotherTree* parent, T* object)
        : ChildBrotherTree(parent, nullptr, nullptr, object)
    {

    }

    template<typename T>
    ChildBrotherTree<T>::ChildBrotherTree(T* object)
        : ChildBrotherTree(nullptr, nullptr, nullptr, object)
    {

    }

    template<typename T>
    ChildBrotherTree<T>::ChildBrotherTree() 
        : ChildBrotherTree(nullptr, nullptr, nullptr, nullptr)
    {

    }

    template<typename T>
    ChildBrotherTree<T>::~ChildBrotherTree()
    {
        _parent = nullptr;
        _child = nullptr;
        _brother = nullptr;
        _object = nullptr;
    }

    template<typename T>
    inline void ChildBrotherTree<T>::SetObject(T* object)
    {
        _object = object;
    }

    template<typename T>
    inline bool ChildBrotherTree<T>::IsParentValid()
    {
        return _parent;
    }

    template<typename T>
    inline bool ChildBrotherTree<T>::IsChildValid()
    {
        return _child;
    }

    template<typename T>
    inline bool ChildBrotherTree<T>::IsBrotherValid()
    {
        return _brother;
    }

    template<typename T>
    inline typename ChildBrotherTree<T>& ChildBrotherTree<T>::Parent()
    {
        return *_parent;
    }

    template<typename T>
    inline typename ChildBrotherTree<T>& ChildBrotherTree<T>::Child()
    {
        return *_child;
    }

    template<typename T>
    inline typename ChildBrotherTree<T>& ChildBrotherTree<T>::Brother()
    {
        return *_brother;
    }

    template<typename T>
    inline typename T& ChildBrotherTree<T>::Object()
    {
        return *_object;
    }

    template<typename T>
    inline void ChildBrotherTree<T>::AddChild(ChildBrotherTree<T>& child)
    {
        child._parent = this;
        if (this->_child)
        {
            this->_child->AddBrother(child);
        }
        else
        {
            this->_child = &child;
        }
    }

    template<typename T>
    inline void ChildBrotherTree<T>::AddBrother(ChildBrotherTree<T>& brother)
    {
        brother._parent = this->_parent;
        ChildBrotherTree<T>* b = this->_brother;
        if (b)
        {
            while (b->_brother)
            {
                b = b->_brother;
            }
            b->_brother = &brother;
        }
        else
        {
            this->_brother = &brother;
        }
    }

    template<typename T>
    inline typename ChildBrotherTree<T>& ChildBrotherTree<T>::Remove()
    {
        ChildBrotherTree<T>* result = nullptr;
        if (this->_parent)
        {
            ChildBrotherTree<T>* pre = nullptr;
            ChildBrotherTree<T>* o = this->_parent;
            for (Iterator start = o->GetChildIterator(); start.IsValid(); ++start)
            {
                if (start._node == this)
                {
                    break;
                }
                pre = start._node;
            }
            if (pre)
            {
                pre->_brother = this->_brother;
                this->_parent = nullptr;
                this->_brother = nullptr;
            }
            else
            {
                this->_parent->_child = this->_brother;
            }
            this->_parent = nullptr;
            this->_brother = nullptr;
            result = this;
        }
        else
        {
            this->_parent = nullptr;
            this->_brother = nullptr;
            result = this;
        }

        return *result;
    }

    template<typename T>
    inline typename ChildBrotherTree<T>::Iterator ChildBrotherTree<T>::GetChildIterator()
    {
        return ChildBrotherTree<T>::Iterator(this);
    }

    template<typename T>
    inline typename ChildBrotherTree<T>::Iterator ChildBrotherTree<T>::GetBrotherIterator()
    {
        return ChildBrotherTree<T>::Iterator(this->_parent);
    }
    template<typename T>
    inline ChildBrotherTree<T>::Iterator::Iterator(ChildBrotherTree<T>* node)
        : _node(node ? node->_child : nullptr)
    {

    }
    template<typename T>
    inline ChildBrotherTree<T>::Iterator::Iterator()
        : _node(nullptr)
    {
    }
    template<typename T>
    inline ChildBrotherTree<T>::Iterator::~Iterator()
    {
    }
    template<typename T>
    inline bool ChildBrotherTree<T>::Iterator::IsValid()
    {
        return _node;
    }
    template<typename T>
    inline typename ChildBrotherTree<T>& ChildBrotherTree<T>::Iterator::Node()
    {
        return *_node;
    }
    template<typename T>
    inline typename ChildBrotherTree<T>::Iterator ChildBrotherTree<T>::Iterator::operator++()
    {
        _node = _node->_brother;
        return *this;
    }
}
