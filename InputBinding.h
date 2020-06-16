/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/10
Description : Various class/struct definitions 
to be used by the Inputhandling InputSystem class
----------------------------------------------*/
#ifndef INPUTBINDING_H
#define INPUTBINDING_H

// std inclusions
#include <string>
#include <array>
#include <vector>

#include <windows.h>

namespace Input {
    
    // Enumeration of different GameCommands
    enum class GameCommands
    {
        Quit,
        MoveForward,
        MoveBackward,
        MoveLeft,
        MoveRight,
        CameraRotation
    };

    // Enum to emphasize the different states of a key
    enum class KeyState
    {
        StillReleased,
        JustPressed,
        StillPressed,
        JustReleased
    };

    // Wrapping struct keycode and above enum
    struct Binding
    {
    private:
        unsigned int keyCode;         // Windows Keycode
        KeyState keyState;            // Associated keystate

    public:
        Binding();
        Binding(const unsigned int pkeyCode, const KeyState pkeyState);
        ~Binding() {};

        friend class InputSystem;
    };

    // Maps a game command to a Binding
    struct Chord
    {
    private:
        std::wstring name;            // Human readable command name
        std::vector<Binding> chord;   // Sequence of keys mapped to "m_Name" command

    public:
        Chord();
        Chord(const std::wstring& pName, const unsigned int pKeyCode, const KeyState pKeyState);
        Chord(const std::wstring& pName, const Binding& pChord);
        Chord(const std::wstring& pName, const std::vector<Binding>& plChord);
        ~Chord() {};

        // Accessors for member variables
        std::vector<Binding>& GetChord() { return chord; }
        std::wstring&         GetName()  { return name;  }
    };
}

#endif