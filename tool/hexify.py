import sys
import getopt

# 0-9 A-Z a-z
hex_digits = range(48, 58) + range(65, 71) + range(97, 103)

def hexify_bin_to_lines(bs, width):
    ss = []
    s = ""
    prev_escaped = False
    for b in bs:
        o = ord(b)
        if ((o >= 32 and o <= 126 and o != 34 and o != 92) and
            (prev_escaped == False or o not in hex_digits)):
            # a printable character except for " \
            # prev_escaped is used for preventing a compiler from
            # treating 0x12 + a as one character 0x12a which is wrong.
            c = b
            prev_escaped = False
        elif o == 0:
            # shortcut of zero
            c = "\\0"
            prev_escaped = True
        else:
            # escaped one for a non-printable character
            c =  "\\x{0:02x}".format(o)
            prev_escaped = True

        if len(s) + len(c) > width:
            ss.append(s)
            s = ""
        s += c

    if len(s) > 0:
        ss.append(s)

    return ss


def print_hexified_bin_as_c(bs, width, indent=""):
    ss = hexify_bin_to_lines(bs, width - 2)
    for s in ss:
        print '{0}"{1}"'.format(indent, s)


def usage():
    print("hexify [options] .egt")
    print "  -w width : specify max width of line excluding an indent. (default: 80)"
    print "  -i str   : specify indent of each line. (for tab use \\t. default: empty)"


def main():
    argv = sys.argv[1:]
    if len(argv) == 0:
        usage()
        sys.exit(2)

    opts, args = getopt.getopt(argv, "w:i:")
    if len(args) != 1:
        usage()
        sys.exit(2)

    bin = open(args[0], "rb").read()
    width = 80
    indent = ""

    for o, a in opts:
        if o == "-w":
            width = int(a)
        elif o == "-i":
            indent = a.replace("\\t", "\t")

    print_hexified_bin_as_c(bin, width, indent)


if __name__ == "__main__":
    main()
