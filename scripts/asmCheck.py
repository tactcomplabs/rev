import argparse

parser = argparse.ArgumentParser(description="Match SST output with asm file - use with __REV_DEEP_TRACE__ define")
parser.add_argument('-a', '--asmFile', dest='asmFilename', required=True)
parser.add_argument('-s', '--sstOut', dest='sstOutfile', required=True)
args = parser.parse_args()

try:
    sst_out = open(args.sstOutfile, 'r')
except Exception:
    print("Cannot open file " + args.sstOutfile)
    exit(1)

sstLines = sst_out.readlines()

try:
    asm = open(args.asmFilename, 'r')
except Exception:
    print("Cannot open file " + args.asmFilename)
    exit(1)

asmLines = asm.readlines()

asmHash = {}
# Strips the newline character
for line in asmLines:
    splitLine = line.strip().split(":")
    PC = splitLine[0]
    if "<" in PC:
        continue
    asmHash[PC] = line.strip()

for line in sstLines:
    if "RDT:" in line:
        PC = line.split("PC = ")[1].split(' ')[0]
        so = line.split("Inst:")[1].strip()
        if PC in asmHash:
            o = '{:<60} {:<3} {:<60}'.format(asmHash[PC].strip(), ":::", so)
            print(o)
        else:
            print("WARNING: PC " + PC + " not found in asm file, but was executed by Rev")
