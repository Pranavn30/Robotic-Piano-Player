import serial
import time
import struct
import mido
import os

# Set up the serial connection
Serial = serial.Serial(port="COM51", baudrate=9600, bytesize=8, parity=serial.PARITY_NONE, stopbits=1, timeout=1)

# Note_ON and Note_OFF command MIDI values
note_on = 144
note_off = 128

def send_message(cmd, note, velocity):
    Serial.write(struct.pack('!BBB', cmd, note, velocity))

# Change the working directory
dir = r'N:'
os.chdir(dir)

# Load the MIDI file
mid = mido.MidiFile('test.mid', clip=True)

for m in mid.tracks[0]:
    # Check if note_on or note_off message
    if m.type == 'note_on':
        velocity = m.velocity
        if velocity > 0:  # actual note_on event
            print('Note started:', m)
            send_message(note_on, m.note, velocity)
            time.sleep(0.1)  # optional, just to space out the notes a bit
        else:  # note_off disguised as note_on
            print('Note stopped:', m)
            send_message(note_off, m.note, velocity)
            time.sleep(0.1)  # optional
    elif m.type == 'note_off':
        print('Note stopped:', m)
        send_message(note_off, m.note, m.velocity)
        time.sleep(0.1)  # optional
