from shtab_parser import get_offsets_by_type
from parser import *
from shtab_parser import get_name

symtab_offsets = get_offsets_by_type(2)


def get_value(x):
    return x


def get_type(x):
    match x:
        case 0:
            t = "NOTYPE"
        case 1:
            t = "OBJECT"
        case 2:
            t = "FUNC"
        case 3:
            t = "SECTION"
        case 4:
            t = "FILE"
        case 5:
            t = "COMMON"
        case 6:
            t = "TLS"
        case 10:
            t = "LOOS"
        case 12:
            t = "HIOS"
        case 13:
            t = "LOPROC"
        case 15:
            t = "HIPROC"
        case _:
            t = "UNKNOWN"
    return t


def get_bind(x):
    match x:
        case 0:
            b = "LOCAL"
        case 1:
            b = "GLOBAL"
        case 2:
            b = "WEAK"
        case 10:
            b = "LOOS"
        case 12:
            b = "HIOS"
        case 13:
            b = "LOWPROC"
        case 15:
            b = "HIPROC"
        case _:
            b = "UNKNOWN"
    return b


def get_visibility(x):
    match x:
        case 0:
            v = "DEFAULT"
        case 1:
            v = "INTERNAL"
        case 2:
            v = "HIDDEN"
        case 3:
            v = "PROTECTED"
        case 4:
            v = "EXPORTED"
        case 5:
            v = "SINGLETON"
        case 6:
            v = "ELIMINATE"
        case _:
            v = "UNKNOWN"
    return v


def get_index(x):
    match x:
        case 0:
            index = "UNDEF"
        case 0xff00:
            index = "LORESERVE"
        case 0xff01:
            index = "AFTER"
        case 0xff02:
            index = "AMD64_LCOMMON"
        case 0xff1f:
            index = "HIPROC"
        case 0xff20:
            index = "LOOS"
        case 0xff3f:
            index = "LOSUNW"
        case 0xfff1:
            index = "ABS"
        case 0xfff2:
            index = "COMMON"
        case 0xffff:
            index = "XINDEX"
        case _:
            index = x
    return index


def parse_symtab():
    idx = symtab_offsets[0]
    symtab = []
    while idx < symtab_offsets[0] + symtab_offsets[1]:
        symtab_entry = dict()
        symtab_entry["name"] = get_name(get(idx, 4))
        symtab_entry["value"] = get_value(get(idx + 4, 4))
        symtab_entry["size"] = get(idx + 8, 4)
        info = get(idx + 12, 1)
        symtab_entry["type"] = get_type(info & 0xf)
        symtab_entry["bind"] = get_bind(info >> 4)
        symtab_entry["vis"] = get_visibility(get(idx + 13, 1))
        symtab_entry["index"] = get_index(get(idx + 14, 2))
        idx += 16
        symtab.append(symtab_entry)
    return symtab
