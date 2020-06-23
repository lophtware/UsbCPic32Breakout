#
# Hardware:
# Just the breakout board.
#
# Purpose:
# Enables the Slave Transaction Notification Reports and listens for transactions.
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

def send_report_and_wait_for_ack(dev, report):
	dev.write(report)
	while True:
		ack = dev.read(1024)
		if len(ack) == 0 or ack[0] != 0x01 or ack[1] != report[0]:
			continue

		if ack[2] != 0:
			raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))

		break

	return ack[2:]

def enable_notification_reports(dev):
	report_id = 0x0a
	flags = 0x03
	return send_report_and_wait_for_ack(dev, [report_id, flags])

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
		print("Enabling Slave Transaction Notification Reports...")
		enable_notification_reports(dev)

		print("Waiting for notifications; access the device as an I2C Slave...")
		while True:
			report = dev.read(1024, timeout_ms=500)
			if len(report) == 0 or report[0] != 0x0c:
				continue

			register_address = (report[4] << 8) | report[3]
			bank = (report[4] >> 2) & 0x3f
			bank_offset = register_address & 0x3ff
			print("Notified: [%s]" % ", ".join("0x{:02x}".format(x) for x in report))
			print("\tType            :", "Write" if (report[1] & 0x80) != 0 else "Read")
			print("\tDevice Address  : 0x{:02x}".format(report[2]))
			print("\tRegister Address: 0x{:04x}, bank=0x{:02x}, offset=0x{:03x}".format(register_address, bank, bank_offset))
			print("\tNumber of Bytes :", (report[6] << 8) | report[5])
			print("")

	finally:
		dev.close()
