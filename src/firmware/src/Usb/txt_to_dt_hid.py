#
# Utility to convert HID Report Descriptors in a .txt format (ie. comma-
# separated 0x?? values, one item per line) into the .hid format required
# by the USB-IF dt.exe tool.  The conversion isn't 100% (the 0x09 manipulation
# misses some magic which looks like some sort of 'usage stack'), but otherwise
# works.
#
# Couldn't find documentation for .hid, so pieced it together from this:
#   https://github.com/jdunne525/DTImporter/blob/master/Form1.frm
#
# Usage:
#   python3 txt_to_dt_hid.py in.txt out.hid
#   cat in.txt | python3 txt_to_dt_hid.py out.hid
#   cat in.txt | python3 txt_to_dt_hid.py > out.hid
#

import sys
import re

def str2int(s):
	if s == '':
		return 0

	if s.startswith('0x'):
		return int(s, base=16)

	return int(s, base=10)

if __name__ == "__main__":
	try:
		if len(sys.argv) > 2:
			fd_in = open(sys.argv[1], "rt")
			fd_out = open(sys.argv[2], "wb")
		elif len(sys.argv) == 1:
			fd_in = sys.stdin
			fd_out = open(sys.argv[1], "wb")
		else:
			fd_in = sys.stdin
			fd_out = sys.stdout

		junk_regex = re.compile('([ \t\v]+)|(//.*$)', re.MULTILINE)
		items = list(filter(None, junk_regex.sub('', fd_in.read()).splitlines()))

		fd_out.write(bytearray([
			0x22, 0x00, 0x01, 0x00, 0xfe, 0xca,
			len(items) & 0xff, (len(items) >> 8) & 0xff,
			0x0a] + [0x00] * 25))

		usage_page = 0x00
		for item in items:
			item = list(str2int(x) for x in item.split(','))
			if item[0] == 0x05:
				usage_page = item[1]
			elif item[0] == 0x09:
				item = item + [0x00] + [usage_page]

			fd_out.write(bytearray(item + [0x00] * (10 - len(item))))

	finally:
		if fd_in is not None:
			fd_in.close()

		if fd_out is not None:
			fd_out.close()
