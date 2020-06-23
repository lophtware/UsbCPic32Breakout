#
# Hardware:
# Just the breakout board.
#
# Purpose:
# Listens for messages from I2C Masters sent by them writing to the I2C USB
# Bank.
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

		print("Waiting for I2C Masters to send some messages...")
		while True:
			report = dev.read(16, timeout_ms=500)
			if len(report) == 0 or report[0] != 0x10:
				continue

			payload = report[1:]
			print("Received: [%s]" % ", ".join("0x{:02x}".format(x) for x in report))
			print("\tPayload: [%s]" % ", ".join("0x{:02x}".format(x) for x in payload))
			print("")

	finally:
		dev.close()
