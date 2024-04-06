import sys
from symtab_parser import parse_symtab
from text_parser import parse_text

orig_stdout = sys.stdout
f = open(sys.argv[2], 'w')
sys.stdout = f


def printf(format, *args):
    sys.stdout.write(format % args)

print(".text")
parse_text()
print()
print()
print(".symtab")
print()
symtab = parse_symtab()


print("Symbol Value              Size Type     Bind     Vis       Index Name")
for i in range(len(symtab)):
    printf("[%4i] 0x%-15X %5i %-8s %-8s %-8s %6s %s\n", i, symtab[i]["value"],
          symtab[i]["size"], symtab[i]["type"], symtab[i]["bind"],
          symtab[i]["vis"], symtab[i]["index"], symtab[i]["name"])
