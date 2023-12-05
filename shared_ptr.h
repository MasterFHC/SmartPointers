#ifndef SHARED_PTR
#define SHARED_PTR

template <typename T>
class SharedPtr
{
public:
    SharedPtr()
    {
        _p = nullptr;
    }
    SharedPtr(T *p)
    {
        _p = p;
        if (p != nullptr)
            _cnt = new int(1);
    }
    ~SharedPtr()
    {
        release();
    }
    SharedPtr(const SharedPtr &other)
    {
        if (other._p != nullptr)
        {
            _p = other._p;
            _cnt = other._cnt;
            (*_cnt)++;
        }
        else
            _p = nullptr;
    }
    SharedPtr &operator=(const SharedPtr &other)
    {
        if (other._p == _p)
            return *this;
        if (_p != nullptr)
            release();
        if (other._p != nullptr)
        {
            _p = other._p;
            _cnt = other._cnt;
            (*_cnt)++;
        }
        return *this;
    }
    operator bool() const
    {
        if (_p == nullptr)
            return false;
        return true;
    }
    size_t use_count() const
    {
        if (_cnt == nullptr)
            return 0;
        else
            return (*_cnt);
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
        if (_p == nullptr)
            return;
        else
            release();
    }
    void reset(T *p)
    {
        if (_p == p)
            return;
        if (_p != nullptr)
            release();
        if (p != nullptr)
        {
            _p = new T(*p);
            delete p;
            // _p = p;
            _cnt = new int(1);
            /*
             *Huge credit to fellow classmate LiangChen who spotted that I did not delete the newly created T(*p)
             *In addition, writing _p = p also passes the Online Judge, which is not reasonable!!
             *Consider this circumstance: Shared_ptr p1 and p2 both point to the same address,
             *then p2.reset(p1.get()) would cause potential double deletion when reaching the end of the program
             */
        }
    }

private:
    T *_p;
    int *_cnt;
    void release()
    {
        if (_p != nullptr and --(*_cnt) == 0)
        {
            delete _p;
            delete _cnt;
        }
        _p = nullptr;
        _cnt = nullptr;
    }
};

template <typename T, typename... Ts>
SharedPtr<T> make_shared(Ts &&...args)
{ // https://blog.csdn.net/wodownload2/article/details/117512062
    SharedPtr<T> ret(new T(std::forward<Ts>(args)...));
    return ret;
}

#endif // SHARED_PTR