#include "DisplayController.h"
#include <Arduino.h>

constexpr DisplayController::NodeNeighbours DisplayController::NODE_NEIGHBOURS[NumNodes];
constexpr u8 DisplayController::NODE_PINS[NumNodes];

void DisplayController::update(const u32 currentTs, JoystickController& joystickController)
{
    static constexpr u32 SELECTED_BLINK_INTERVAL = 350;
    static constexpr Bitfield16 ALL_SEGMENTS_OFF_MASK = 1 << Node::DP;

    const auto joystickDir = joystickController.getDirection();
    const auto joyPressType = joystickController.getButtonValue(currentTs);
    const bool joyPressed = joyPressType != JoystickController::Press::None;

    const Bitfield16 nodeMask = 1 << currentNode;
    switch (currentState) {
    case State::Disengaged: {
        /* Handle directional input */
        currentNode = NODE_NEIGHBOURS[currentNode][u8(joystickDir)];

        /* Handle button input */
        if (joyPressType == JoystickController::Press::Short)
            currentState = State::Engaged;
        else if (joyPressType == JoystickController::Press::Long) {
            nodeStates &= ALL_SEGMENTS_OFF_MASK;
            currentNode = Node::DP;
        }

        /* Calculate the current node's appropriate blink phase */
        const bool oddInterval = (currentTs / SELECTED_BLINK_INTERVAL) % 2;

        /* Odd interval -> treat node as ON. Even interval -> treat node as OFF */
        const Bitfield16 intervalNodeStates
            = (nodeStates & ~nodeMask) | (oddInterval << currentNode);

        drawNodes(intervalNodeStates);
        break;
    }
    case State::Engaged:
        if (joystickDir == JoystickController::Direction::Left)
            nodeStates ^= nodeMask; /* Toggle the current node */

        if (joyPressed)
            currentState = State::Disengaged;

        drawNodes(nodeStates);
        break;
    default:
        UNREACHABLE;
    }
}

void DisplayController::init() const
{
    for (auto pin : NODE_PINS)
        pinMode(pin, OUTPUT);
}

void DisplayController::drawNodes(const Bitfield16 nodeStates)
{
    for (u32 i = 0; i < NumNodes; ++i) {
        const Bitfield16 mask = 1 << i;
        digitalWrite(NODE_PINS[i], bool(nodeStates & mask));
    }
}