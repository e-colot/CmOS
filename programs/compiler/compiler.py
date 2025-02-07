import sys
import struct

R8 = 1
R16 = 2
IMM8 = 3
IMM16 = 4

REG_8BIT = {
   "FLAGS": 0x0, 
   "R1": 0x1, 
   "R2": 0x2, 
   "R3": 0x3, 
   "R4": 0x4, 
   "R5": 0x5, 
   "RL": 0x6, 
   "RH": 0x7
}
REG_16BIT = {
   "R16": 0x6, 
   "RSI": 0x8, 
   "RDI": 0xA, 
   "RI": 0xC, 
   "RS": 0xE
}

REG = REG_8BIT | REG_16BIT

COMMANDS = {
  "AND": [[0x00, [R8, R8, R8]], [0x01, [R8, R8, IMM8]]],
  "OR": [[0x02, [R8, R8, R8]], [0x03, [R8, R8, IMM8]]],
  "NOT": [[0x04, [R8, R8]]],
  "SHL": [[0x05, [R8, R8, R8]], [0x05, [R16, R8, R16]], [0x06, [R16, R16, IMM8]], [0x06, [R8, R8, IMM8]]],
  "SHR": [[0x07, [R8, R8, R8]], [0x07, [R16, R8, R16]], [0x08, [R16, R16, IMM8]], [0x08, [R8, R8, IMM8]]],  
  "ADD": [[0x10, [R16, R16, R16]], [0x10, [R8, R8, R8]], [0x11, [R16, R16, IMM8]], [0x11, [R8, R8, IMM8]]],
  "SUB": [[0x12, [R16, R16, R16]], [0x12, [R8, R8, R8]], [0x13, [R16, R16, IMM8]], [0x13, [R8, R8, IMM8]], [0x13, [IMM8, R8, R8]]],
  "MUL": [[0x15, [R8, R8]], [0x16, [R8, IMM8]]],
  "IDIV": [[0x17, [R8, R8]], [0x18, [IMM8, R8]]],
  "MOD": [[0x19, [R8, R8]], [0x1A, [IMM8, R8]]],
  "MOV": [[0x20, [R16, R16]], [0x20, [R8, R8]], [0x21, [R8, IMM8]], [0x22, [IMM16]], [0x22, [IMM8]]],
  "LOAD": [[0x23, [R8]], [0x24, []]],
  "STORE": [[0x25, [R8]], [0x26, []]],
  "REGDUMP": [[0x27, []]],
  "REGFILL": [[0x28, []]],
  "CMP": [[0x30, [R16, R16]], [0x30, [R8, R8]]],
  "TEST": [[0x31, [R16, R8]], [0x31, [R8, R8]], [0x32, [R16, IMM8]], [0x32, [R8, IMM8]]],
  "SKIFZ": [[0x40, []]],
  "SKIFNZ": [[0x41, []]],
  "PRNT": [[0xF0, [R16]], [0xF0, [R8]]],
  "HLT": [[0xFF, []]]
}

def loadFile(filenameIn, filenameOut):
  # data will store all the values stored in memory for the program
  with open("../" + filenameIn) as file:
    data_bytes = []
    code_lines = []
    for line in file:
        if line == '\n':
            #empty lines get discarded
            continue
        cmd = cleanLine(line)
        if (cmd[0] == "DB"):
            if (len(cmd) != 2 or not cmd[1].isdigit()):
                raise ValueError(f"Invalid DB statement: {cmd}")
            data_bytes.append(int(cmd[1]))
        else:
            code_lines.append(translateLine(cmd))
    append_to_binary_file(filenameOut, data_bytes, code_lines)

def cleanLine(line):
  a = line.strip()
  b = a.split(" ")
  c = []
  for i in b:
    if i != "":
      c.append(i)
  return c

def translateLine(line):
    output = [0, 0, 0]
    if (line[0] in COMMANDS.keys()):
        cmd = COMMANDS.get(line[0])
        args = argType(line[1:])
        for cmdVariant in cmd:
            if args == cmdVariant[1]:
                # Command found
                output[0] = cmdVariant[0]
                if (len(args) >= 2 and args[0] <= R16 and args[1] <= R16):
                    # starts with R8/16 R8/16
                    output[1] = 16*REG.get(line[1])+REG.get(line[2])
                    if (len(args) == 2):
                        output[2] = 0
                    elif(len(args) >= 3 and args[2] <= R16):
                        output[2] = REG.get(line[3])
                    elif(len(args) >= 3 and args[2] == IMM8):
                        # R8/16 R8/16 IMM8
                        output[2] = int(line[3])
                    else:
                        raise Exception("Unknown parameters") 
                elif(len(args) >= 1 and args[0] == IMM8):
                    # starts with IMM8
                    output[1] = int(line[1])
                    if(len(args) >= 3 and args[1] <= R16 and args[2] <= R16):
                        output[2] = 16*REG.get(line[2])+REG.get(line[3])
                    elif(len(args) >= 2 and args[1] <= R16):
                        output[2] = REG.get(line[2])
                    elif(len(args) == 1):
                        output[1] = 0
                        output[2] = int(line[1])
                    else:
                        raise Exception("Unknown parameters") 
                elif(len(args) >= 1 and args[0] <= R16):
                    # starts with * R8/16
                    output[1] = REG.get(line[1])
                    if(len(args) == 1):
                        output[2] = 0
                    elif(len(args >= 2 and args[1] == IMM8)):
                        output[2] = int(line[2])
                    else:
                        raise Exception("Unknown parameters") 
                elif(len(args) == 0):
                    output[1] = 0
                    output[2] = 0
                elif(len(args) == 1 and args[0] == IMM16):
                    output[1] = int(line[1])//16
                    output[2] = int(line[1])%16
                else:
                    raise Exception("Unknown parameters") 
    else:
        raise Exception("Unknown instruction") 
    return output
    
def argType(arg_list):
    result = []
    for arg in arg_list:
        if arg in REG_8BIT.keys():
            result.append(R8)
        elif arg in REG_16BIT.keys():
            result.append(R16)
        elif arg.isdigit():
            value = int(arg)
            if 0 <= value <= 255:
                result.append(IMM8)
            elif 256 <= value <= 65535:
                result.append(IMM16)
            else:
                raise ValueError(f"Immediate value out of range: {arg}")
        else:
            raise ValueError(f"Unknown argument: {arg}")
    return result

def append_to_binary_file(filename, data, code):
    with open("../" + filename, "ab") as file:  # Open in append binary mode
        # Write the length of data as a 16-bit value
        instructionPtr = [len(data)//256, len(data)%256]
        file.write(bytes(instructionPtr))
        # Write each byte in data
        file.write(bytes(data))
        # Write each instruction (list of 3 integers)
        for instruction in code:
            file.write(struct.pack("BBB", *instruction))

if __name__ == "__main__":
  input = sys.argv[1]
  output = sys.argv[2]
  loadFile(input, output)

