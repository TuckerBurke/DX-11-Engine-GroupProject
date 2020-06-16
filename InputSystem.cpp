/*----------------------------------------------
Ruben Young (rubenaryo@gmail.com)
Date : 2019/10
Description : InputSystem method definitions
----------------------------------------------*/
#include "InputSystem.h"
#include <cstdio>

namespace Input {

    // Init all keyboard states to 0
    InputSystem::InputSystem()
    {
        keyboardCurrent.fill(0);
        keyboardPrevious.fill(0);
        mousePrevious.x = 0;
        mousePrevious.y = 0;
        mouseCurrent.x = 0;
        mouseCurrent.y = 0;

        SetDefaultKeyMap();
    }

    // Release all dynamic memory
    InputSystem::~InputSystem()
    {
        // Release keyMap and clear
        for (auto pair : keyMap)
            delete pair.second;
        keyMap.clear();

        // No need to delete dynamic memory from activeKeyMap, it's already deleted in keymap*
        activeKeyMap.clear();
    }

    void InputSystem::Frame(float dt, Camera* camera)
    {
        float speed = camera->GetMovementSpeed() * dt;
        
        GetKeyboardInput();

        // Camera references
        Transform* playerTransform = camera->GetTransform();

        // Act on user input:
        // - Iterate through all active keys
        // - Check for commands corresponding to activated chords
        // - Do something based on those commands
        for (auto pair : activeKeyMap)
        {
            switch (pair.first)
            {
            case GameCommands::Quit:
                PostQuitMessage(0);
                break;
            case GameCommands::MoveForward:
                camera->GetTransform()->MoveRelative(0.0f, 0.0f, speed);
                break;
            case GameCommands::MoveBackward:
                camera->GetTransform()->MoveRelative(0.0f, 0.0f, -speed);
                break;
            case GameCommands::MoveLeft:
                camera->GetTransform()->MoveRelative(-speed, 0.0f, 0.0f);
                break;
            case GameCommands::MoveRight:
                camera->GetTransform()->MoveRelative(speed, 0.0f, 0.0f);
                break;
            case GameCommands::CameraRotation:
            {
                std::pair<float, float> delta = this->GetMouseDelta();

                float mouseSensitivity = camera->GetSensitivity();
                delta.first *= mouseSensitivity * dt;
                delta.second *= mouseSensitivity * dt;
                
                // Rotate Camera based on mouse coord delta
                playerTransform->Rotate(delta.second, delta.first, 0.0f);

                // Avoid mouse "lockback" by discarding current delta.
                // We've already moved with it, so we don't want it to read Windows' snap back as false user input.
                #ifndef DEBUG
                mouseCurrent = mousePrevious;
                #endif
                break;
            }
            }
        }

        // Lock player movement to the floor
        DirectX::XMFLOAT3 currPos = playerTransform->GetPosition();
        playerTransform->SetPosition(currPos.x, 2.1f, currPos.z);
        
        UpdateMouseState();
    }

    // Stores previous/current keymappings (from windows)
    // Updates active keymap by comparing all chords 
    // Updates mouse mapping
    void InputSystem::GetKeyboardInput()
    {
        // Get the keyboard state from windows
        GetKeyboardState();

        // Update active/non-active keymaps
        UpdateKeymaps();
    }

    void InputSystem::OnMouseMove(short newX, short newY)
    {
        mouseCurrent = { newX, newY };
    }

    std::pair<float,float> InputSystem::GetMouseDelta() const
    {
        std::pair<float, float> pt;
        
        pt.first  = static_cast<float>(mouseCurrent.x - mousePrevious.x);
        pt.second = static_cast<float>(mouseCurrent.y - mousePrevious.y);
        return pt;
    }

    // Clears active key map, then fills it with all values from keyMap with a 'fulfilled' chord
    void InputSystem::UpdateKeymaps()
    {
        // Reset active key map
        activeKeyMap.clear();

        // Map which keys are active into the active key map
        for (auto key : keyMap)
        {
            bool activeKey = true;

            // Test Chord
            for (Binding binding : key.second->GetChord())
            {
                if (GetKeyboardKeyState(binding.keyCode) != binding.keyState)
                {
                    activeKey = false;
                    break;
                }
            }

            // Passed Chord Check : move key to active key map
            if (activeKey)
                activeKeyMap.insert(std::pair<GameCommands, Chord*>(key.first, key.second));
        }
    }

    void InputSystem::UpdateMouseState()
    {
        mousePrevious = mouseCurrent;
    }

    // Stores the current state of the keyboard as 'previous'
    // then reads in new 'current values from windows
    void InputSystem::GetKeyboardState()
    {
        keyboardPrevious = keyboardCurrent;

        for (int i = 0; i < 256; i++)
            keyboardCurrent[i] = isPressed(i);
    }

    // Use logic to deduce Keystate from current and previous keyboard states
    const KeyState InputSystem::GetKeyboardKeyState(const unsigned int pkeyCode) const
    {
        if (keyboardPrevious[pkeyCode] == 1)
            if (keyboardCurrent[pkeyCode] == 1)
                return KeyState::StillPressed;  // true, true
            else
                return KeyState::JustReleased;  // true, false
        else
            if (keyboardCurrent[pkeyCode] == 1)
                return KeyState::JustPressed;   // false, true
            else
                return KeyState::StillReleased; // false, false
    }
    
    // Set the default key bindings with human readable names
    void InputSystem::SetDefaultKeyMap()
    {
        keyMap.clear();
        keyMap[GameCommands::Quit]         = new Chord(L"Quit", VK_ESCAPE, KeyState::JustReleased);
        keyMap[GameCommands::MoveForward]  = new Chord(L"Move Forward", 'W', KeyState::StillPressed);
        keyMap[GameCommands::MoveBackward] = new Chord(L"Move Backward", 'S', KeyState::StillPressed);
        keyMap[GameCommands::MoveLeft]     = new Chord(L"Move Left", 'A', KeyState::StillPressed);
        keyMap[GameCommands::MoveRight]    = new Chord(L"Move Right", 'D', KeyState::StillPressed);

        keyMap[GameCommands::CameraRotation] = new Chord(L"Camera Rotation", VK_RBUTTON, KeyState::StillPressed);
    }
}