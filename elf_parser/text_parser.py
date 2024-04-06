import sys

from shtab_parser import get_offsets_by_name
from parser import get
from symtab_parser import parse_symtab
from shtab_parser import get_addr

orig_stdout = sys.stdout
f = open(sys.argv[2], 'w')
sys.stdout = f


def printf(format, *args):
    sys.stdout.write(format % args)


# M extension
m = ["mul", "mulh", "mulhsu", "mulhu", "div", "divu", "rem", "remu"]
# I-type, funct7 == 0
i0 = ["add", "sll", "slt", "sltu", "xor", "srl", "or", "and"]
# B-type
b = ["beq", "bne", "", "", "blt", "bge", "bltu", "bgeu"]
# I-type load instructions
load = ["lb", "lh", "lw", "", "lbu", "lhu", "", ""]
# S-type
store = ["sb", "sh", "sw", "", "", "", "", ""]


def print_command(args):
    addr = args[0]
    hex_code = args[1]
    name = args[2]
    if name == "invalid_instruction":
        printf("   %05x:\t%08x\t%-7s\n",
               addr, hex_code, "invalid_instruction")
    elif name == "jal":
        printf("   %05x:\t%08x\t%7s\t%s, 0x%x <%s>\n", addr,
               hex_code, name, args[3], args[4], args[5])
    elif name in b:
        printf("   %05x:\t%08x\t%7s\t%s, %s, 0x%x, <%s>\n", addr,
               hex_code, name, args[3], args[4], args[5], args[6])
    elif name in load or name in store or name == "jalr":
        printf("   %05x:\t%08x\t%7s\t%s, %d(%s)\n",addr,
               hex_code, name, args[3], args[4], args[5])
    elif name == "fence":
        printf("   %05x:\t%08x\t%7s\t%s, %s\n",
               addr, hex_code, "fence", args[3], args[4])
    elif len(args) == 6:
        printf("   %05x:\t%08x\t%7s\t%s, %s, %s\n", addr,
               hex_code, name, args[3], args[4], args[5])
    elif len(args) == 5:
        printf("   %05x:\t%08x\t%7s\t%s, %s\n", addr,
               hex_code, name, args[3], args[4])
    elif len(args) == 3:
        printf("   %05x:\t%08x\t%7s\n", addr,
               hex_code, name)


def print_tag(addr, tag_name):
    printf("\n%08x \t<%s>:\n", addr, tag_name)


def print_invalid():
    printf("   %05x:\t%08x\t%-7s\n", 0, 0, "invalid_instruction")


text_offsets = get_offsets_by_name(".text")
reg = ['0'] * 32
l = []


def abi_reg():
    reg[0] = "zero"
    reg[1] = "ra"
    reg[2] = "sp"
    reg[3] = "gp"
    reg[4] = "tp"
    for i in range(5, 8):
        reg[i] = "t" + str(i - 5)
    reg[8] = "s0"
    reg[9] = "s1"
    for i in range(10, 18):
        reg[i] = "a" + str(i - 10)
    for i in range(18, 28):
        reg[i] = "s" + str(i - 16)
    for i in range(28, 32):
        reg[i] = "t" + str(i - 25)


def extract(command, left, right):
    return (command >> right) & (1 << left - right + 1) - 1


def parse_r(command):
    funct7 = extract(command, 31, 25)
    funct3 = extract(command, 14, 12)
    rs1 = extract(command, 19, 15)
    rs2 = extract(command, 24, 20)
    rd = extract(command, 11, 7)
    opcode = extract(command, 6, 0)
    name = ""
    if opcode == 0b0110011:
        if funct7 == 1:
            name = m[funct3]
        elif funct7 == 0:
            name = i0[funct3]
        elif funct7 == 0b0100000:
            if funct3 == 0:
                name = "sub"
            elif funct3 == 0b101:
                name = "sra"
    elif opcode == 0b0010011:
        if funct7 == 0:
            if funct3 == 0b001:
                name = "slli"
            elif funct3 == 0b101:
                name = "srli"
        elif funct7 == 0b0100000 and funct3 == 0b101:
            name = "srai"
        if name != "":
            return [name, reg[rd], reg[rs1], rs2]
    if name == "":
        return ["invalid_instruction"]
    else:
        return [name, reg[rd], reg[rs1], reg[rs2]]


def two_comp(num, size):
    if (num >> size - 1) & 1 == 1:
        num -= 1 << size
    return num


def parse_i(command):
    imm = two_comp(extract(command, 31, 20), 12)
    opcode = extract(command, 6, 0)
    funct3 = extract(command, 14, 12)
    rs1 = extract(command, 19, 15)
    rd = extract(command, 11, 7)
    name = ""
    if opcode == 0b0000011:
        name = load[funct3]
    elif opcode == 0b0010011:
        if funct3 == 0b000:
            name = "addi"
        elif funct3 == 0b010:
            name = "slti"
        elif funct3 == 0b011:
            name = "sltiu"
        elif funct3 == 0b100:
            name = "xori"
        elif funct3 == 0b110:
            name = "ori"
        elif funct3 == 0b111:
            name = "andi"
    elif opcode == 0b1100111:
        name = "jalr"
    if name == "":
        return ["invalid_instruction"]
    elif name == "jalr":
        return [name, reg[rd], imm, reg[rs1]]
    elif name in load:
        return [name, reg[rd], imm, reg[rs1]]
    else:
        return [name, reg[rd], reg[rs1], imm]


def parse_s(command):
    upper_imm = extract(command, 31, 25)
    lower_imm = extract(command, 11, 7)
    imm = two_comp((upper_imm << 5) + lower_imm, 12)
    funct3 = extract(command, 14, 12)
    rs1 = extract(command, 19, 15)
    rs2 = extract(command, 24, 20)
    name = store[funct3]
    if name == "":
        return ["invalid_instruction"]
    else:
        return [name, reg[rs2], imm, reg[rs1]]
    return


def find_symtab_name(addr):
    for el in symtab:
        if el["value"] == addr and el["type"] == "FUNC":
            return el["name"]
    return False


def find_local_name(tag):
    for i in range(len(l)):
        if l[i] == tag:
            return "L" + str(i)
    return False


def get_tag_name(imm):
    tag_name = find_symtab_name(imm)
    if not tag_name:
        tag_name = find_local_name(imm)
        if not tag_name:
            l.append(imm)
            tag_name = "L" +str(len(l) - 1)
    return tag_name


def get_imm_12(command, addr):
    bit_12 = extract(command, 31, 31)
    bit_11 = extract(command, 7, 7)
    bits_10_5 = extract(command, 30, 25)
    bits_4_1 = extract(command, 11, 8)
    imm = (bit_12 << 12) + (bit_11 << 11) + (bits_10_5 << 5) + (bits_4_1 << 1)
    imm = two_comp(imm, 13)
    return imm + addr


def parse_b(command, addr):
    imm = get_imm_12(command, addr)
    rs1 = extract(command, 19, 15)
    rs2 = extract(command, 24, 20)
    funct3 = extract(command, 14, 12)
    name = b[funct3]
    if name == "":
        return ["invalid_instruction"]
    else:
        return [name, reg[rs1], reg[rs2], imm, get_tag_name(imm)]


def to_two_comp(imm, size):
    imm = abs(imm)
    imm = ~imm
    imm = (imm + 1) % (1 << size)
    return imm


def parse_u(command):
    imm = extract(command,31, 12)
    rd = extract(command, 11, 7)
    opcode = extract(command, 6, 0)
    name = ""
    if opcode == 0b0110111:
        name = "lui"
        imm = two_comp(imm, 20)
        if imm < 0:
            imm = to_two_comp(imm, 32)
    elif opcode == 0b0010111:
        name = "auipc"
    if name == "":
        return ["invalid_instruction"]
    else:
        return [name, reg[rd], hex(imm)]


def get_imm_20(command, addr):
    bit_20 = extract(command, 31, 31)
    bits_19_12 = extract(command, 19, 12)
    bit_11 = extract(command, 20, 20)
    bits_10_1 = extract(command, 30, 21)
    imm = ((bit_20 << 20) + (bits_19_12 << 12) +
           (bit_11 << 11) + (bits_10_1 << 1))
    imm = two_comp(imm, 21)
    return imm + addr


def parse_j(command, addr):
    imm = get_imm_20(command, addr)
    rd = extract(command, 11, 7)
    name = "jal"
    if name == "":
        return ["invalid_instruction"]
    else:
        return [name, reg[rd], imm, get_tag_name(imm)]


def parse_special(command):
    funct = extract(command, 31, 20)
    funct3 = extract(command, 14, 12)
    rs1 = extract(command, 19, 15)
    rd = extract(command, 11, 7)
    name = ""
    if funct3 == 0 and rs1 == 0 and rd == 0:
        if funct == 0:
            name = "ecall"
        elif funct == 1:
            name = "ebreak"
    if name == "":
        return ["invalid_instruction"]
    else:
        return [name]


def parse_fence(command):
    zeroes = extract(command, 31, 28)
    funct3 = extract(command, 14, 12)
    if zeroes != 0 or funct3 != 0:
        print_invalid()
    else:
        l = ['i', 'o', 'r', 'w']
        p = [0] * 4
        s = [0] * 4
        p[0] = extract(command, 27, 27)
        p[1] = extract(command, 26, 26)
        p[2] = extract(command, 25, 25)
        p[3] = extract(command, 24, 24)

        s[0] = extract(command, 23, 23)
        s[1] = extract(command, 22, 22)
        s[2] = extract(command, 21, 21)
        s[3] = extract(command, 20, 20)
        pred = ""
        succ = ""
        for i in range(4):
            pred += l[i] * p[i]
            succ += l[i] * s[i]
        return ["fence", pred, succ]


def parse_command(command, addr):
    opcode = extract(command, 6, 0)
    if opcode == 0b0000011:
        return parse_i(command)
    elif opcode == 0b0010011:
        funct3 = extract(command, 14, 12)
        if funct3 in [0b001, 0b101]:
            return parse_r(command)
        else:
            return parse_i(command)
    elif opcode == 0b0010111:
        return parse_u(command)
    elif opcode == 0b0100011:
        return parse_s(command)
    elif opcode == 0b0110011:
        return parse_r(command)
    elif opcode == 0b0110111:
        return parse_u(command)
    elif opcode == 0b1100011:
        return parse_b(command, addr)
    elif opcode == 0b1100111:
        return parse_i(command)
    elif opcode == 0b1101111:
        return parse_j(command, addr)
    elif opcode == 0b1110011:
        return parse_special(command)
    elif opcode == 0b0001111:
        return parse_fence(command)
    else:
        printf("   %05x:\t%08x\t%-7s\n", 0, 0, "invalid_instruction")


def fill_l(command, addr):
    opcode = extract(command, 6, 0)
    name = b[extract(command, 14, 12)]
    if opcode == 0b1100011 and name != "":
        imm = get_imm_12(command, addr)
        get_tag_name(imm)
    elif opcode == 0b1101111:
        imm = get_imm_20(command, addr)
        get_tag_name(imm)



def parse_text():
    abi_reg()
    global symtab
    symtab = parse_symtab()
    addr = get_addr(".text")
    for i in range(text_offsets[0], text_offsets[0] + text_offsets[1], 4):
        fill_l(get(i, 4), addr)
        addr += 4
    addr = get_addr(".text")
    for i in range(text_offsets[0], text_offsets[0] + text_offsets[1], 4):
        if find_symtab_name(addr):
            print_tag(addr, find_symtab_name(addr))
        if find_local_name(addr):
            print_tag(addr, find_local_name(addr))
        command = get(i, 4)
        p_args = parse_command(command, addr)
        args = [addr, command, *p_args]
        print_command(args)
        addr += 4
