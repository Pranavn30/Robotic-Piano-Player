import serial
import time
import struct
import mido
import os

# Set up the serial connection
#port = serial.Serial(port="COM51", baudrate=9600, bytesize=8, parity=serial.PARITY_NONE, stopbits=1, timeout=1)

def send_message(note, start_time, stop_time):
    port.write(struct.pack('!BBB', note, start_time, stop_time))

# Load the MIDI file
song = mido.MidiFile(r"C:\Users\User\Desktop\A_UMICH\SEM1\EECS 373\final proj\C_major_scale.mid")
#song = mido.MidiFile(r"C:\Users\User\Desktop\A_UMICH\SEM1\EECS 373\final proj\Under-The-Sea.mid")

type_list = []
note_list = []
tim_list = []

formated_data = []

for m in song: #record data in arrays
    tim_list.append(m.time)
    type_list.append(m.type)

    #if it is a note_on or note_off we want the note info
    if(m.type =='note_on' or m.type == 'note_off'):
        note_list.append(m.note)
    else:
        note_list.append('null')    
    
tim_sum = 0

for i in range(len(tim_list)): #rearrange data
    #Trasnforming time to absolute values
    tim_sum = tim_list[i] + tim_sum

    #Build formated array
    formated_data.append([type_list[i], note_list[i], tim_sum])
    

def_data1 = []
def_data2 = []
j = 0
for m in formated_data:
    #print(i)
    if(m[0] =='note_on' or m[0] == 'note_off'):
        def_data1.append([m[0], m[1], m[2]])
        #print(def_data1[j])
        j+=1    
    
for i in range(int(len(def_data1)/2)):
    j = 2*i
    def_data2.append([def_data1[j][1], def_data1[j][2], def_data1[j+1][2]])
    print(def_data1[j][1], def_data1[j][2], def_data1[j+1][2])
    
    #send_message(def_data1[j][1], def_data1[j][2], def_data1[j+1][2])








    

    