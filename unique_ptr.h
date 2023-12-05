#ifndef UNIQUE_PTR
#define UNIQUE_PTR

template <typename T>
class UniquePtr
{
public:
    UniquePtr()
    {
        _p = nullptr;
    }
    explicit UniquePtr(T *pointer) noexcept
    {
        _p = pointer;
    }
    ~UniquePtr()
    {
        delete _p;
        _p = nullptr;
    }

    // The normal copy constructor and the copy assignment operator are deleted
    UniquePtr(UniquePtr &other) = delete;
    UniquePtr &operator=(UniquePtr &other) = delete;

    // Try these: move copy constructor and move assignemnt operator!
    UniquePtr(UniquePtr &&other)
    {
        _p = other._p;
        other._p = nullptr;
    }
    UniquePtr &operator=(UniquePtr &&other) noexcept
    {
        if (&other == this)
            return *this;
        delete _p;
        _p = other._p;
        other._p = nullptr;
        return *this;
    }

    operator bool() const
    {
        if (_p == nullptr)
            return false;
        else
            return true;
    }
    T *get() const
    {
        return _p;
    }
    T &operator*() const
    {
        return *_p;
    }
    T *operator->() const
    {
        return _p;
    }
    void reset()
    {
        delete _p;
        _p = nullptr;
    }
    void reset(T *new_pointer)
    {
        if (new_pointer == _p)
            return;
        delete _p;
        _p = new_pointer;
    }
    T *release()
    {
        T *ret = _p;
        _p = nullptr;
        return ret;
    }

private:
    T *_p;
};

// A tricky question: to implement the make_unique function
template <typename T, typename... Ts>
UniquePtr<T> make_unique(Ts &&...args)
{
    UniquePtr<T> ret(new T(std::forward<Ts>(args)...));
    return ret;
}

#endif // UNIQUE_PTR