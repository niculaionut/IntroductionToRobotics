/*
 *  Simple method for playing note sequences on a buzzer.
 *  In this case: Little Fugue in G Minor (BWV 578).
 */

#include "common/music.h"

static MelodyPlayer mc(MASS_IN_B_MINOR, 15000);

void setup() { pinMode(3, OUTPUT); }

void loop() { mc.play(3); }

int main()
{
    init();
    setup();
    for (;;)
        loop();
}
