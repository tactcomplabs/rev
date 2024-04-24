import sys
import argparse

parser = argparse.ArgumentParser(description="Match SST output with asm file and/or print call stack at a specified clock tick - use with TRACER_ON in target program.  Use 'riscv64-unknown-elf-objdump -dC -Mno-aliases <exe>' to create assembly file")
parser.add_argument('-a', '--asmFile', dest='asmFilename', required=True)
parser.add_argument('-s', '--sstOut', dest='sstOutfile', required=True)
parser.add_argument('-c', '--callStk', dest='callStackClk', type=int, required=False)
args = parser.parse_args()

try:
  sst_out = open(args.sstOutfile, 'r')
except:
  print("Cannot open file " + args.sstOutfile)
  exit(1)

sstLines = sst_out.readlines()

try:
  asm = open(args.asmFilename, 'r')
except:
  print("Cannot open file " + args.asmFilename)
  exit(1)

asmLines = asm.readlines()

asmHash = {}
asmFunctStartHash = {}
asmFunctRetHash = {}
curFunction = ""
firstInst = False
prevPC = ""
# Strips the newline character
for line in asmLines:
  if ">:" in line:
    asmFunctRetHash[prevPC] = curFunction
    curFunction = line.strip()
    firstInst = True
    continue
  splitLine = line.strip().split(":")
  PC = splitLine[0]
  prevPC = PC
  if "<" in PC:
    continue
  asmHash[PC] = line.strip()
  if firstInst:
      asmFunctStartHash[PC] = curFunction
      firstInst = False

printCS = False
if(args.callStackClk):
    printCS = True

callStack = ["main"]

for line in sstLines:
  if "Render:" in line and "Core" in line:
    print(line)
    clk = line.split("Render:")[1].split(']:')[0].strip()
    PC = line.split("*I ")[1].split(':')[0].split('0x')[1]
    so = line.split("*I ")[1].split('\t')[0].strip()
    if PC in asmHash:
      o = '{:<60} {:<3} {:<60}'.format(asmHash[PC].strip(),":::", so)
      if(not args.callStackClk):
        print(o)
    else:
      print("WARNING: PC " + PC + " not found in asm file, but was executed by Rev")

    if int(clk) == args.callStackClk:
        print("Call Stack at time: " + clk + " and PC: " + PC)
        for f in callStack:
            print(f)

    if PC in asmFunctStartHash:
        callStack.append(asmFunctStartHash[PC])

    if PC in asmFunctRetHash:
        callStack.pop()


