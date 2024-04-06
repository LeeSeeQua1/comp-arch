from parser import *


def get_offsets_by_index(i):
    idx = e_shoff + i * e_shentsize
    return [get(idx + 16, 4), get(idx + 20, 4)]


def get_offsets_by_type(t):
    idx = e_shoff
    while idx < e_shoff + shtab_size:
        type = get(idx + 4, 4)
        if type == t:
            return [get(idx + 16, 4), get(idx + 20, 4)]
        idx += e_shentsize


def get_section_name(idx):
    name = ""
    idx += get_offsets_by_index(e_shstrndx)[0]
    while data[idx] != 0:
        name += chr(data[idx])
        idx += 1
    return name


def get_offsets_by_name(n):
    idx = e_shoff
    while idx < e_shoff + shtab_size:
        name = get_section_name(get(idx, 4))
        if name == n:
            return [get(idx + 16, 4), get(idx + 20, 4)]
        idx += e_shentsize


def get_addr(name):
    idx = e_shoff
    while idx < e_shoff + shtab_size:
        name_ = get_section_name(get(idx, 4))
        if name == name_:
            return get(idx + 12, 4)
        idx += e_shentsize


strtab_offsets = get_offsets_by_name(".strtab")


def get_name(idx):
    if idx == 0:
        return ""
    name = ""
    idx += strtab_offsets[0]
    while data[idx] != 0:
        name += chr(data[idx])
        idx += 1
    return name
