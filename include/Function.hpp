#pragma once

#include "Femto"

template<typename Func_t>
class Function {

    template<typename Type> struct helper;

    template<typename Ret, typename ... Args>
    struct helper<Ret(Args...)> {
        using ptr = Ret (*) (uptr, Args...);
    };

    using Func = typename helper<Func_t>::ptr;

    uptr data = 0;
    Func func = nullptr;

public:
    constexpr Function() = default;

    constexpr Function(Function&& other) :
        data(other.data),
        func(other.func) {}

    constexpr Function(const Function &other) :
        data(other.data),
        func(other.func) {}

    constexpr Function(Function* other) :
        data(other->data),
        func(other->func) {}

    constexpr Function(uptr data, Func func) :
        data{data},
        func{func} {}

    constexpr Function(Func_t* unwrapped) :
        data{reinterpret_cast<uptr>(unwrapped)},
        func{[](uptr data, auto ... args){
            return reinterpret_cast<Func_t*>(data)(std::forward<decltype(args)>(args)...);
        }} {}

    constexpr Function& operator = (Function&& other) {
        data = other.data;
        func = other.func;
        return *this;
    }

    constexpr Function& operator = (const Function& other) {
        data = other.data;
        func = other.func;
        return *this;
    }

    template <typename Class>
    constexpr Function(const Class* obj) :
        data{reinterpret_cast<uptr>(obj)},
        func{[](uptr data, auto ... args){
            return (*reinterpret_cast<Class*>(data))(std::forward<decltype(args)>(args)...);
        }} {}

    template <typename Class, typename std::enable_if<!std::is_pointer<Class>::value, int>::type = 0>
    constexpr Function(const Class& obj) : Function(&obj) {}

    template <typename ... Args>
    auto operator () (Args&& ... args)  const {
        return func(data, std::forward<Args>(args)...);
    }

    operator bool () const {
        return func != nullptr;
    }
};
