#
# Hardware:
# Just the breakout board.
#
# Purpose:
# Queries the first device for the I2C Master configuration.
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

def read_configuration_as_report(dev):
	report_id = 0x06
	return dev.get_input_report(report_id, 27)

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

		report = read_configuration_as_report(dev)

		print("\nReport: [%s]" %
			", ".join("0x{:02x}".format(x) for x in report))

		print("  Baud Rate     :", (report[11] << 8) |  report[10])
		print("  Address ACK   :", 10 * ((report[14] << 8) | report[13]), "ms")
		print("  Slave Data ACK:", 10 * ((report[17] << 8) | report[16]), "ms")
		print("  Slave Data In :", 10 * ((report[20] << 8) | report[19]), "ms")
		print("  Master ACK    :", 10 * ((report[23] << 8) | report[22]), "ms")
		print("  Stop Bit      :", 10 * ((report[26] << 8) | report[25]), "ms")

	finally:
		dev.close()
