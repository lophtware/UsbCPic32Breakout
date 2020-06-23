#
# Hardware:
# Just the breakout board.
#
# Purpose:
# Allows getting and setting the Slave's ROM contents.
#

import sys
import hid

VID = 0x1209
PID = 0xcb0b
I2C_INTERFACE = 0x04

def get_path_to_device_interface(intf):
	for ls in hid.enumerate():
		if ls["vendor_id"] == VID and ls["product_id"] == PID and ls["interface_number"] == intf:
			return ls["path"]

	return ""

def send_read_rom(dev):
	report_id = 0x1e
	return dev.get_input_report(report_id, 1037)

def send_write_rom(dev, contents, offset):
	report_id = 0x1e
	unlock_key = [0x00] * 8
	padded_contents = contents[0:1024] if len(contents) >= 1024 else contents + [0x00] * (1024 - len(contents))
	return dev.write(
		[report_id] +
		unlock_key +
		[offset & 0xff, (offset >> 8) & 0xff,
		len(contents) & 0xff, (len(contents) >> 8) & 0xff] +
		padded_contents)

def str_to_int(s):
	if s.startswith("0x"):
		return int(s[2:], base=16)
	elif s.startswith("0b"):
		return int(s[2:], base=2)
	else:
		return int(s, base=10)

def do_read(dev, offset, count, filename):
	print("Reading ROM [0x{:03x}, 0x{:03x}]...".format(offset, offset + count - 1))
	report = send_read_rom(dev)
	contents = report[13 + offset:13 + offset + count]

	print("Using %s to write %d bytes." % (filename, count))
	with open(filename, "wb") as fd:
		contents = fd.write(bytearray(contents))

def do_write(dev, offset, count, filename):
	print("Using %s to read %d bytes." % (filename, count))
	with open(filename, "rb") as fd:
		contents = bytearray(fd.read())[0:count]

	try:
		print("Writing ROM [0x{:03x}, 0x{:03x}]...".format(offset, offset + count - 1))
		send_write_rom(dev, list(contents), offset)
		ack = dev.read(16)
		if len(ack) > 0:
			print("Did not expect a response: [%s]" % ", ".join("0x{:02x}".format(x) for x in ack))

	except OSError:
		print("Looks like the device was reset, so done.")

	return

if __name__ == "__main__":
	i2c_interface_path = get_path_to_device_interface(I2C_INTERFACE)
	if not i2c_interface_path:
		print("Device is not connected.")
		exit(1)

	dev = hid.device()
	dev.open_path(i2c_interface_path)
	try:
		print("Manufacturer :", dev.get_manufacturer_string())
		print("Product      :", dev.get_product_string())
		print("Serial Number:", dev.get_serial_number_string())
		print("")

		if len(sys.argv) != 5:
			print("Usage: slave-rom-contents.py <read|write> <offset> <count> <filename>")
		elif sys.argv[1] == "write":
			do_write(dev, str_to_int(sys.argv[2]), str_to_int(sys.argv[3]), sys.argv[4])
		else:
			do_read(dev, str_to_int(sys.argv[2]), str_to_int(sys.argv[3]), sys.argv[4])

	finally:
		dev.close()
