#!/usr/bin/env python
#
# test_midiin_callback.py
#
"""Shows how to receive MIDI input by setting a callback function."""

from sys import argv, exit
from time import sleep
from rtmidi.midiutil import open_midiport
from serial import Serial

micro_periods = [
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        30578, 28861, 27242, 25713, 24270, 22909, 21622, 20409, 19263, 18182, 17161, 16198,  # C1 - B1
        15289, 14436, 13621, 12856, 12135, 11454, 10811, 10205, 9632, 9091, 8581, 8099,  # C2 - B2
        7645, 7218, 6811, 6428, 6068, 5727, 5406, 5103, 4816, 4546, 4291, 4050,  # C3 - B3
        3823, 3609, 3406, 3214, 3034, 2864, 2703, 2552, 2408, 2273, 2146, 2025,  #C4 - B4
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]


def new_midi(port, serial):
    try:
        midi_in, midi_port = open_midiport(port)
    except (EOFError, KeyboardInterrupt):
        exit()

    def cb(event, data=None):
        msg, dt = event
        status = msg[0]
        data = msg[1:]
        channel = status & 0x0f
        cmd = status & 0xf0

        if cmd == 144:  # note on
            serial.write(chr(channel))
            note = data[0]
            speriod = micro_periods[note] / (40 * 2)
            serial.write(chr(speriod >> 8))
            serial.write(chr(speriod & 0xff))

        if cmd == 128:  # note off
            serial.write(chr(channel))
            serial.write("\x00\x00")

        # print("[%s] ch:%d cmd:%d %r" % (midi_port, channel, cmd, data))

    print("Attaching MIDI input callback handler.")
    midi_in.set_callback(cb)

    return midi_in


def new_serial(port):
    return Serial(port, 9600, timeout=5)


serial_name = argv[1] if argv[1:] else "/dev/ttyACM0"

serial = new_serial(serial_name)
midi_in = new_midi(0, serial)

print("Entering main loop. Press Control-C to exit.")
try:
    while True:
        sleep(1)
except KeyboardInterrupt:
    pass
finally:
    midi_in.close_port()
    del midi_in
