#pragma once

#include <variant>
#include "Femto"

template<class... States>
class StateMachine {

    std::variant<States...> state;

    void (*_update)(StateMachine *) = [](StateMachine *sm){
        std::get<0>(sm->state).update();
    };

    template<class First, class... Rem>
    void report_rec(s32 i){
        LOG("State ", i, ": ", sizeof(First), " bytes\n");
        if constexpr (sizeof...(Rem)) {
            report_rec<Rem...>(i+1);
        }
    }

public:
    void report(){
        LOG("Free RAM: ", getFreeRAM(), " bytes\n");
        report_rec<States...>(0);
    }

    void update(){ _update(this); }

    template<class StateType>
    void setState(){
        _update = [](StateMachine *sm){
            auto& state = sm->state.template emplace<StateType>();
            sm->_update = [](StateMachine *sm){
                std::get<StateType>(sm->state).update();
            };
            state.init();
            sm->_update(sm);
        };
    }

    template<class StateType>
    StateType &getState() {
        return std::get<StateType>(state);
    }
};

template <typename Type>
class State {
public:
    void init(){}
    static Type& get();
};
