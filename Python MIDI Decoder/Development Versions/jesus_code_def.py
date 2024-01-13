import serial
import time
import struct
import mido

# Set up the serial connection
port = serial.Serial(port="COM7", baudrate=9600, bytesize=8, parity=serial.PARITY_NONE, stopbits=1, timeout=1)

def send_message(note, start_time_h, start_time_l, stop_time_h,stop_time_l):
    port.write(struct.pack('!BBBBB', note, start_time_h, start_time_l, stop_time_h,stop_time_l))

# Load the MIDI file
song = mido.MidiFile(r"C:\Users\User\Desktop\A_UMICH\SEM1\EECS 373\final proj\C_major_scale.mid")
#song = mido.MidiFile(r"C:\Users\User\Desktop\A_UMICH\SEM1\EECS 373\final proj\Rollinginthedeep.mid")

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
    #Transforming time to absolute values
    tim_sum = tim_list[i] + tim_sum

    #Build formated array
    formated_data.append([type_list[i], note_list[i], tim_sum])
    

sel_data = [] #only data note_on or note_off included
def_data = [] #data in format note, start_time, stop_time
j = 0

#Keep only note_on or note_off commands
for m in formated_data:
    if(m[0] =='note_on' or m[0] == 'note_off'):
        sel_data.append([m[0], m[1], m[2]])
        j+=1    

i = 0 
while(sel_data != []):
    item_st = sel_data[0] #0 --> type || 1 --> note || 2--> time

    for d in sel_data:
        if((d[1] == item_st[1]) & (d[0] == 'note_off')): #we found matching note
            item_end = d
            break
    
    #save to def data and erase
    def_data.append([item_end[1],item_st[2],item_end[2]])  #format --> note - start_time - end_time
    sel_data.remove(item_st)
    sel_data.remove(item_end)

    i+=1

for a in def_data:
    #begining message
    if a == def_data[0]:
        send_message(0,0,0,0,0)
    
    note = int(a[0])
    start_tim = int(a[1]*1000)
    stop_tim = int(a[2]*1000)

    print(note, start_tim, stop_tim)

    #formating upper lower bytes
    send_message(note, (start_tim >> 8) & 255, start_tim & 255, (stop_tim >> 8) & 255, stop_tim & 255)

    #end song message
    if a == (def_data[len(def_data) - 1]):
        send_message(255,0,0,0,0)
    







    

    