#include <TimerOne.h>

#define PIN_LOOP(_var) for(byte _var=2; _var<=11; _var+=2)
#define RESOLUTION 40 //Microsecond resolution for notes


/*NOTE: Many of the arrays below contain unused indexes.  This is
  to prevent the Arduino from having to convert a pin input to an alternate
  array index and save as many cycles as possible.  In other words information
  for pin 2 will be stored in index 2, and information for pin 4 will be
  stored in index 4.*/


/*An array of maximum track positions for each step-control pin.  Even pins
  are used for control, so only even numbers need a value here.  3.5" Floppies have
  80 tracks, 5.25" have 50.  These should be doubled, because each tick is now
  half a position (use 158 and 98).
 */
byte MAX_POSITION[] = {
    0,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0};

//Array to track the current position of each floppy head.  (Only even indexes (i.e. 2,4,6...) are used)
byte current_position[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*Array to keep track of state of each pin.  Even indexes track the control-pins for toggle purposes.  Odd indexes
  track direction-pins.  LOW = forward, HIGH=reverse
 */
int current_state[] = {
    0,0,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW
};

//Current period assigned to each pin.  0 = off.  Each period is of the length specified by the RESOLUTION
//variable above.  i.e. A period of 10 is (RESOLUTION x 10) microseconds long.
unsigned int current_period[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//Current tick
unsigned int current_tick[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};



//Setup pins (Even-odd pairs for step control and direction
void setup() {
    PIN_LOOP(p) {
        pinMode(p,   OUTPUT); // Step control i
        pinMode(p+1, OUTPUT); // Direction i
    }

    Timer1.initialize(RESOLUTION); // Set up a timer at the defined resolution
    Timer1.attachInterrupt(tick); // Attach the tick function

    Serial.begin(9600);

    reset_all();
    digitalWrite(13, LOW);
}


void loop() {
    //Only read if we have

    if (Serial.available() > 2) {
        //Watch for special 100-message to reset the drives
        if (Serial.peek() == 100) {
            digitalWrite(13, HIGH);
            reset_all();
            //Flush any remaining messages.
            while(Serial.available() > 0) {
                Serial.read();
            }
        }
        else {
            current_period[Serial.read()] = (Serial.read() << 8) | Serial.read();
        }
    }
}


/*
   Called by the timer inturrupt at the specified resolution.
 */
void tick() {
    /*
       If there is a period set for control pin 2, count the number of
       ticks that pass, and toggle the pin if the current period is reached.
     */
    PIN_LOOP(p) {
        if (current_period[p] > 0) {
            current_tick[p]++;
            if (current_tick[p] >= current_period[p]) {
                toggle_pin(p, p+1);
                current_tick[p]=0;
            }
        }
    }
}

void toggle_pin(byte pin, byte direction) {
    //Switch directions if end has been reached
    if (current_position[pin] >= MAX_POSITION[pin]) {
        current_state[direction] = HIGH;
        digitalWrite(direction,HIGH);
    }
    else if (current_position[pin] <= 0) {
        current_state[direction] = LOW;
        digitalWrite(direction,LOW);
    }

    //Update current_position
    if (current_state[direction] == HIGH) {
        current_position[pin]--;
    }
    else {
        current_position[pin]++;
    }

    //Pulse the control pin
    digitalWrite(pin,current_state[pin]);
    current_state[pin] = ~current_state[pin];
}


//Resets all the pins
void reset_all() {
    //Stop all notes (don't want to be playing during/after reset)
    PIN_LOOP(p) {
        current_period[p] = 0; // Stop playing notes
    }

    // New all-at-once reset
    for (byte s=0; s<80;s++) { // For max drive's position
        PIN_LOOP(p) {
            digitalWrite(p+1,HIGH); // Go in reverse
            digitalWrite(p,HIGH);
            digitalWrite(p,LOW);
        }
        delay(5);
    }

    PIN_LOOP(p) {
        current_position[p] = 0; // We're reset.
        digitalWrite(p+1,LOW);
        current_state[p+1] = 0; // Ready to go forward.
    }
}
