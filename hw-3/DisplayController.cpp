#include "DisplayController.h"

constexpr DisplayController::NodeNeighbours DisplayController::NODE_NEIGHBOURS[NumNodes];
constexpr u8 DisplayController::NODE_PINS[NumNodes];

void DisplayController::init()
{
    for (auto pin : NODE_PINS)
        pinMode(pin, OUTPUT);

    currentNode = Node::DP;
}

void DisplayController::update(const u32 currentTs, JoystickController& joystickController)
{
    static constexpr u32 SELECTED_BLINK_INTERVAL = 256;
    static constexpr Bitset8 ALL_NODES_OFF = 0;

    const auto joystickDir = joystickController.getDirection();
    const auto joyPress = joystickController.getButtonValue(currentTs);

    const auto nodeMask = Bitset8(1 << currentNode);
    switch (currentState) {
    case State::Disengaged: {
        /* Handle directional input */
        currentNode = NODE_NEIGHBOURS[currentNode][u8(joystickDir)];

        /* Handle button input */
        if (joyPress == JoystickController::Press::Short)
            currentState = State::Engaged;
        else if (joyPress == JoystickController::Press::Long) {
            nodeStates = ALL_NODES_OFF;
            currentNode = Node::DP;
        }

        /* Calculate the current node's appropriate blink phase */
        const bool oddInterval = (currentTs / SELECTED_BLINK_INTERVAL) % 2;

        /* Odd interval -> treat node as ON. Even interval -> treat node as OFF */
        const auto intervalNodeStates
            = Bitset8((nodeStates & ~nodeMask) | (oddInterval << currentNode));

        drawNodes(intervalNodeStates);
        break;
    }
    case State::Engaged:
        if (joystickDir == JoystickController::Direction::Left
            || joystickDir == JoystickController::Direction::Right)
            nodeStates ^= nodeMask; /* Toggle the current node */

        if (joyPress != JoystickController::Press::None)
            currentState = State::Disengaged;

        drawNodes(nodeStates);
        break;
    default:
        UNREACHABLE;
    }
}

void DisplayController::drawNodes(const Bitset8 nodeStates)
{
    for (u32 i = 0; i < NumNodes; ++i) {
        const auto nodeValue = u8((nodeStates >> i) & 1);
        digitalWrite(NODE_PINS[i], nodeValue);
    }
}
