#
# Hardware:
# Just the breakout board.
#
# Purpose:
# Gets and sets the name of the current configuration.
#

import sys
import hid

VID = 0x1209
PID = 0xcb0b
USB_INTERFACE = 0x03

def get_path_to_device_interface(intf):
	for ls in hid.enumerate():
		if ls["vendor_id"] == VID and ls["product_id"] == PID and ls["interface_number"] == intf:
			return ls["path"]

	return ""

def str_to_unicode_bytes(s):
	b = []
	for x in s:
		b = b + [ord(x) & 0xff, (ord(x) >> 8) & 0xff]

	return b

def unicode_bytes_to_str(b):
	return b.decode("utf-16")

def send_report_and_wait_for_ack(dev, report):
	dev.write(report)
	while True:
		ack = dev.read(1024)
		if (ack[0] == 0x01 and ack[1] != report[0]) or (ack[0] != 0x01 and ack[0] != report[0]):
			continue

		if ack[0] == 0x01 and ack[2] != 0:
			raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))

		break

	return ack

def get_configuration_name(dev):
	report_id = 0x04
	return dev.get_input_report(report_id, 34)

def set_configuration_name(dev, name):
	report_id = 0x04
	update_flag = 0x80
	unicode_name = str_to_unicode_bytes(name)[:32]
	if len(unicode_name) < 32:
		unicode_name = unicode_name + [0] * (32 - len(unicode_name))

	return send_report_and_wait_for_ack(dev, [report_id, update_flag] + unicode_name)

if __name__ == "__main__":
	usb_interface_path = get_path_to_device_interface(USB_INTERFACE)
	if not usb_interface_path:
		print("Device is not connected.")
		exit(1)

	dev = hid.device()
	dev.open_path(usb_interface_path)
	try:
		print("Manufacturer :", dev.get_manufacturer_string())
		print("Product      :", dev.get_product_string())
		print("Serial Number:", dev.get_serial_number_string())
		print("")

		if len(sys.argv) < 2:
			report = get_configuration_name(dev)
			print("Report: [%s]" % ", ".join("0x{:02x}".format(x) for x in report))
			print("\tName:", unicode_bytes_to_str(bytearray(report[2:])))
		else:
			report = set_configuration_name(dev, sys.argv[1])
			print("Report: [%s]" % ", ".join("0x{:02x}".format(x) for x in report))

	finally:
		dev.close()
