#!/usr/bin/env python3

import os
import sys

fin = open(sys.argv[1], 'r')
fout = open('out.obj', 'w')
line = fin.readline()
while (line[0] != 'f') :
    fout.write(line)
    line = fin.readline()
while (line != '') :
    splitline = line.split()
    newline = splitline[0] + " " + splitline[1] + "/0/0 " + splitline[2] + "/0/0 " + splitline[3] + "/0/0\n"
    fout.write(newline)
    line = fin.readline()
fin.close()
fout.close()
