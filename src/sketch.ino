#include <TimerOne.h>

#define N_PINS        17
#define RESOLUTION    40  //Microsecond resolution for notes
#define MAX_POSITION 158

#define PIN_LOOP(_var) for(byte _var=0; _var<=N_PINS; _var+=2)


// Array to track the current position of each floppy head.
// (Only even indexes (i.e. 2,4,6...) are used)
byte current_position[N_PINS];

// Array to keep track of state of each pin.
// Even indexes track the control-pins for toggle purposes.
// Odd indexes track direction-pins.
// LOW = forward, HIGH =reverse
byte pin_state[N_PINS];

// Current semi-period assigned to each pin.  0 = off.
// Each semi-period is of the length specified by the RESOLUTION
// variable above.  i.e. A semi-period of 10 is (RESOLUTION x 10) microseconds long.
unsigned int current_speriod[N_PINS];

// Current tick
byte current_tick[N_PINS];


void write_pin(byte pin, byte value) {
    pin_state[pin] = value;
    digitalWrite(pin, value);
}


//Setup pins (Even-odd pairs for step control and direction
void setup() {
    PIN_LOOP(p) {
        pinMode(p,   OUTPUT);  // Step control i
        pinMode(p+1, OUTPUT);  // Direction i
    }

    Timer1.initialize(RESOLUTION); // Set up a timer at the defined resolution
    Timer1.attachInterrupt(tick);  // Attach the tick function

    Serial.begin(9600);
}


void loop() {
    if (Serial.available() > 2) {
        byte channel = Serial.read();
        unsigned int speriod = (Serial.read() << 8) | Serial.read();
        if (channel == 0) {
            PIN_LOOP(p) {
                current_speriod[p] = speriod;
            }
        }
        else {
            current_speriod[channel] = speriod;
        }
    }
}


void tick() {
    // If there is a speriod set for control pin 2, count the number of
    // ticks that pass, and toggle the pin if the current speriod is reached.
    PIN_LOOP(p) {
        if (current_speriod[p] > 0) {
            current_tick[p]++;
            if (current_tick[p] >= current_speriod[p]) {
                toggle_pin(p, p+1);
                current_tick[p] = 0;
            }
        }
    }
}

void toggle_pin(byte pin, byte direction) {
    // Switch directions if end has been reached
    if (current_position[pin] >= MAX_POSITION) {
        write_pin(direction, HIGH);
    }
    else if (current_position[pin] <= 0) {
        write_pin(direction, LOW);
    }

    // Update current_position
    if (pin_state[direction] == HIGH) {
        current_position[pin]--;
    }
    else {
        current_position[pin]++;
    }

    // Pulse the control pin
    write_pin(pin, ~pin_state[pin]);
}

