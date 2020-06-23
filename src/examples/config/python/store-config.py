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
CORE_INTERFACE = 0x00

def get_path_to_device_interface(intf):
	for ls in hid.enumerate():
		if ls["vendor_id"] == VID and ls["product_id"] == PID and ls["interface_number"] == intf:
			return ls["path"]

	return ""

def send_report_and_wait_for_ack(dev, report):
	dev.write(report)
	while True:
		try:
			ack = dev.read(1024)

		except OSError:
			return []

		if (ack[0] == 0x01 and ack[1] != report[0]) or ack[0] != 0x01:
			continue

		if ack[0] == 0x01 and ack[2] != 0:
			raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))

		break

	return ack

def store_configuration(dev, make_boot):
	report_id = 0x08
	unlock_key = [0x00] * 8
	flags = 0x80 if make_boot else 0x00
	return send_report_and_wait_for_ack(dev, [report_id] + unlock_key + [flags])

if __name__ == "__main__":
	core_interface_path = get_path_to_device_interface(CORE_INTERFACE)
	if not core_interface_path:
		print("Device is not connected.")
		exit(1)

	dev = hid.device()
	dev.open_path(core_interface_path)
	try:
		print("Manufacturer :", dev.get_manufacturer_string())
		print("Product      :", dev.get_product_string())
		print("Serial Number:", dev.get_serial_number_string())
		print("")

		report = store_configuration(dev, make_boot=False)
		if len(report) > 0:
			print("Report: [%s]" % ", ".join("0x{:02x}".format(x) for x in report))
		else:
			print("Looks like the device was reset, so done.")

	finally:
		dev.close()
