/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/10
Description : Implementation for structures defined in InputBinding.h
----------------------------------------------*/
#include "InputBinding.h"
#include <vector>
#include <string>

namespace Input {
    // Default Binding Constructor
    Binding::Binding() :
        keyCode(0),
        keyState(KeyState::JustReleased)
    {};

    // Create Binding from keycode and keystate
    Binding::Binding(const unsigned int pkeyCode, const KeyState pkeyState) :
        keyCode(pkeyCode),
        keyState(pkeyState)
    {};

    // Default Chord Constructor
    Chord::Chord() :
        name(L""),
        chord(0)
    {};

    // Create Chord from Name, create binding from keycode and keystate and add it to Chord structure
    Chord::Chord(const std::wstring& pName, const unsigned int pkeyCode, const KeyState pkeyState) :
        name(pName)
    {
        chord.push_back(Binding(pkeyCode, pkeyState));
    }

    // Create Chord from Name and Binding&
    Chord::Chord(const std::wstring& pName, const Binding& pBinding) :
        name(pName)
    {
        chord.push_back(pBinding);
    }

    // Create Chord from Name and vector of Bindings
    Chord::Chord(const std::wstring& pName, const std::vector<Binding>& plBinding) :
        name(pName),
        chord(plBinding)
    {};
}
