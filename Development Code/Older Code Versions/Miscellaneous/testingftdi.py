import serial
import time
import struct

Serial = serial.Serial(port = "COM51", baudrate = 9600, bytesize = 8, parity = serial.PARITY_NONE, stopbits = 1, timeout =1);

velocity = 100#//velocity of MIDI notes, must be between 0 and 127
# //higher velocity usually makes MIDI instruments louder
 
noteON = 144#//144 = 10010000 in binary, note on command
noteOFF = 128#//128 = 10000000 in binary, note off command

string = b''



#//send MIDI message
def MIDImessage(MIDInote, STARTtime,  STOPtime):

    string = b''
    string += struct.pack('!B',MIDInote)
    string += struct.pack('!B',STARTtime)
    string += struct.pack('!B',STOPtime)
   # string += struct.pack('!B',MIDIvelocity)

    Serial.write(string)#//send note on or note off command 
    #Serial.write(MIDInote)#//send pitch data
   # Serial.write(MIDIvelocity)#//send velocity data

    
MIDImessage(50, 0, velocity)#//midimessage(note number, start time, stop time)
time.sleep(1)
MIDImessage(noteOFF, 50, velocity)#//turn note off
time.sleep(1)
MIDImessage(noteON, 60, velocity)#//turn note on
time.sleep(1)
MIDImessage(noteOFF, 60, velocity)#//turn note off
time.sleep(1)
MIDImessage(noteON, 70, velocity)#//turn note on
time.sleep(1)
MIDImessage(noteOFF, 70, velocity)#//turn note off
time.sleep(1)
  
