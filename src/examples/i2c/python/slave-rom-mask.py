#
# Hardware:
# Just the breakout board.
#
# Purpose:
# When invoked with no command-line arguments, prints the device's Protected
# ROM Address mask.
#
# When invoked with an integer argument, sets the device's Protected ROM
# Address mask.
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

def set_protected_rom_address_mask(dev, mask):
	report_id = 0x18
	dev.write([report_id, mask & 0xff, (mask >> 8) & 0xff])
	ack = dev.read(7)
	if ack[0] != 0x01 or ack[1] != report_id:
		raise Exception("Unexpected report ID 0x{0:02x} / 0x{1:02x} when waiting for ACK".format(ack[0], ack[1]))

	return ack[2:]

def get_protected_rom_address_mask(dev):
	report_id = 0x18
	buf = dev.get_input_report(report_id, 3)
	return (buf[2] << 8) | buf[1]

def str_to_int(s):
	if s.startswith("0x"):
		return int(s[2:], base=16)
	elif s.startswith("0b"):
		return int(s[2:], base=2)
	else:
		return int(s, base=10)

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

		if len(sys.argv) == 2:
			mask = str_to_int(sys.argv[1])
			ack = set_protected_rom_address_mask(dev, mask)
			if ack[0] != 0:
				print("Error 0x{:02x} whilst trying to set address mask to 0x{:04x}".format(ack[0], mask))
			else:
				print("Protected ROM Address Mask: 0x{0:04x}".format(mask))

		else:
			mask = get_protected_rom_address_mask(dev)
			print("Protected ROM Address Mask: 0x{0:04x}".format(mask))

	finally:
		dev.close()
