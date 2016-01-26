/* Copyright 2013–2016 Kullo GmbH. All rights reserved. */
#pragma once

// This is a re-implementation of C++11 nested exceptions, made necessary by MSVC++ 2013
#ifdef _MSC_VER

#include <exception>
#include <type_traits>
#include <utility>

namespace std {

class nested_exception
{
public:
    nested_exception() : m_nested(std::current_exception())
    {
    }

    nested_exception(const nested_exception &other) = default;
    virtual ~nested_exception() = default;
    nested_exception &operator=(const nested_exception& other) = default;

    void rethrow_nested() const
    {
        if (m_nested)
        {
            std::rethrow_exception(m_nested);
        }
        else
        {
            std::terminate();
        }
    }

    exception_ptr nested_ptr() const
    {
        return m_nested;
    }

private:
    exception_ptr m_nested;
};

namespace {

template<class T>
class _nested_exception : public T, public nested_exception
{
public:
    explicit _nested_exception(T &&ex)
        : T(forward<T>(ex))
    {
    }
};

}

template<class T>
void throw_with_nested(T &&t)
{
    typedef typename remove_reference<T>::type NoRefT;

    if (is_base_of<nested_exception, NoRefT>::value)
    {
        throw forward<T>(t);
    }
    else
    {
        throw _nested_exception<T>(forward<T>(t));
    }
}

template<class E>
void rethrow_if_nested(const E &e)
{
    const nested_exception *nested = dynamic_cast<const nested_exception*>(&e);
    if (nested) nested->rethrow_nested();
}

}

#endif // _MSC_VER
