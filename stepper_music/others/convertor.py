import os
os.system("./midicomp -t  badapple.mid > badapple.in")
tempo = 0
f = open("badapple.in", "r")
for line in f.readlines():
    if "Tempo" in line:
        tempo = int(line.split(' ')[3])
    
