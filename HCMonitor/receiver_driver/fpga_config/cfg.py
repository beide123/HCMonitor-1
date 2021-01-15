
import os

os.system("lspci -x -d 10ee:6014 > log")

fd = open("log", "r")

lines = fd.readlines()

line = lines[2].split(" ")

print line[3] + " " + line[4]

if line[3] == "00" and line[4] == "00" :
	print "Warning: PCIe BAR = 0 !"
	os.system("../wrbar")
else:
	os.system("./fpga_config")

fd.close()

os.system("rm -f log")
