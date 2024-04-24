import sys
import argparse

parser = argparse.ArgumentParser(description="Match SST output with spkie trace file - use with __REV_DEEP_TRACE__ define")
parser.add_argument('-a', '--asmFile', dest='asmFilename', required=True)
parser.add_argument('-s', '--sstOut', dest='sstOutfile', required=True)
parser.add_argument('-k', '--spikeOut', dest='spikeOutfile', required=True)
parser.add_argument('-v', '--verbose', dest='verbose', required=False)
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

try:
  spike = open(args.spikeOutfile, 'r')
except:
  print("Cannot open file " + args.spikeOutfile)
  exit(1)

spikeLines = spike.readlines()

asmHash = {}
spikeList = []
startPC = 0
startPCFound = False
for line in asmLines:
  splitLine = line.strip().split(":")
  PC = splitLine[0]
  if "<main>:" in line:
    startPCFound = True
    continue
  if "<" in PC:
    continue
  asmHash[PC] = line.strip()
  if startPCFound:
      print("Found start in asm at: " + PC)
      startPC = int(PC,16)
      startPCFound = False

startPCInt = startPC

foundStart = False
for line in spikeLines:
    l = line.split()
    if len(l) < 5:
        continue
    if ('exception' in l[2]) and foundStart :
        spikeList.append(line.strip())
        continue
    if 'exception' in l[2]:
        continue
    PCint = int(l[2],16)
    if PCint == startPCInt:
        foundStart = True
        print("Found start in spike at: " + hex(PCint))

    if foundStart:
        spikeList.append(line.strip())
    else:
        continue

match = '-'

o = '{:<9} {:<70} {:<3} {:<70} {:<3} {:<60}'.format("PC Match", "ASM Instruction" ,":::", "Rev Instruction", ":::", "Spike Instruction")
print(o)
for line in sstLines:
  if "RDT: Executed" in line:
    PC = line.split("PC = ")[1].split(' ')[0]
    so = line.split("Inst:")[1].strip()
    if "page_fault" in spikeList[0]:
        print("found exception")
        spikeList.pop(0)
        spikeList.pop(0)
        while int(PC,16) != int(spikeList[0].split()[2],16):
            spikeList.pop(0)

    if PC in asmHash:
        if int(PC,16) == int(spikeList[0].split()[2],16):
            match = '+'
        else:
            match = '-'
        o = '{:<3} {:<70} {:<3} {:<70} {:<3} {:<60}'.format(match, asmHash[PC].strip(),":::", so, ":::", spikeList.pop(0))
        print(o)
    else:
        print("WARNING: PC " + PC + " not found in asm file, but was executed by Rev")


