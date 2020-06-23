#
# Hardware:
# Just the breakout board.
#
# Purpose:
# Allows getting and setting the Slave's ROM / RAM contents without
# interrupting I2C functionality or requiring a device reset.
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

def send_transaction_report_and_wait_for_ack(dev, report):
	dev.write(report)
	while True:
		ack = dev.read(1024, timeout_ms=500)
		if len(ack) == 0:
			continue

		if ack[0] == report[0]:
			return ack

		if ack[0] == 0x01 and ack[1] == report[0]:
			raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))

def do_transaction(dev, report_id, read_address, read_count, write_address, write_contents, read_then_write=True):
	padded_contents = (write_contents + [0x00] * (32 - len(write_contents))) if len(write_contents) < 32 else write_contents[0:32]
	return send_transaction_report_and_wait_for_ack(dev, [
		report_id,
		0x80 if read_then_write else 0x00,
		read_address & 0xff, (read_address >> 8) & 0xff,
		read_count & 0xff,
		write_address & 0xff, (write_address >> 8) & 0xff,
		len(write_contents) & 0xff] +
		padded_contents)

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
		print("")

		if len(sys.argv) == 9:
			report_id = 0x19 if sys.argv[1] == "rom" else 0x11
			read_then_write = True if sys.argv[2] == "rw" else False
			read_address = min(max(0, str_to_int(sys.argv[3])), 0xffff)
			read_count = min(max(0, str_to_int(sys.argv[4])), 32)
			out_filename = sys.argv[5]
			write_address = min(max(0, str_to_int(sys.argv[6])), 0xffff)
			write_count = min(max(0, str_to_int(sys.argv[7])), 32)
			in_filename = sys.argv[8]

			write_contents = []
			if write_count > 0:
				print("Reading %d bytes from %s..." % (write_count, in_filename))
				with open(in_filename, "rb") as fd:
					write_contents = list(bytearray(fd.read()))[0:write_count]

			print("Attempting a {} of {} {} bytes at 0x{:04x} then a {} of {} bytes at 0x{:04x}...".format(
				"read" if read_then_write else "write",
				read_count if read_then_write else write_count,
				"RAM" if report_id == 0x11 else "ROM",
				read_address if read_then_write else write_address,
				"write" if read_then_write else "read",
				write_count if read_then_write else read_count,
				write_address if read_then_write else read_address
			))

			result = do_transaction(dev, report_id, read_address, read_count, write_address, write_contents, read_then_write)
			print("Result (%s): [%s]" % (
				"SUCCESS" if (result[1] & 0x01) != 0 else "FAILURE",
				", ".join("0x{:02x}".format(x) for x in result)))

			if read_count > 0:
				print("Writing %d bytes to %s..." % (read_count, out_filename))
				with open(out_filename, "wb") as fd:
					fd.write(bytearray(result[8:8 + read_count]))

		else:
			print("Usage: slave-rom-transaction.py <rom|ram> <rw|wr> <r_addr> <r_count> <out_filename> <w_addr> <w_count> <in_filename>")

	finally:
		dev.close()
