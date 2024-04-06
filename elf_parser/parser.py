import sys

file_in = open(sys.argv[1], "rb")
data = bytearray(file_in.read())


def get(pos, length):
    num = 0
    for i in range(length - 1, -1, -1):
        num += data[pos + i] << 8 * i
    return num


e_shoff = get(32, 4)
e_shentsize = get(46, 2)
e_shnum = get(48, 2)
e_shstrndx = get(50, 2)
shtab_size = e_shentsize * e_shnum