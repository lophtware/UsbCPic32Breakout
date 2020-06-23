#
# Hardware:
# Two USB C / PIC32 Breakout Boards connected via I2C.
#
# Purpose:
# Uses the first device as an I2C Master to query the serial number of the
# second device, which wil act as an I2C Slave.
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

def read_serial_number_as_report(dev):
	report_id = 0x80
	out_count = 2 << 5
	in_count = 20
	device_address = 0x31
	register_address = (2 << 10) | 0x001c
	payload = [(register_address >> 8) & 0xff, register_address & 0xff] + [0]*5
	dev.write([
		report_id, out_count,
		in_count & 0xff, (in_count >> 8) & 0xff,
		device_address] +
		payload)

	return dev.read(5 + (in_count | 7))

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

		report = read_serial_number_as_report(dev)
		num_bytes_read = (report[0] - 0x80) * 8 + ((report[1] >> 5) & 0x07)
		remaining = report[2] | (report[3] << 8)
		fault_code = report[1] & 0x03
		faults = ["None", "Address NACK", "Data NACK", "Timeout"]

		print("\nReport: [%s]" %
			", ".join("0x{:02x}".format(x) for x in report))

		print("  Bytes Read     :", num_bytes_read)
		print("  Started Read   ?", "YES" if (report[1] & 0x08) else "NO")
		print("  Collision      ?", "YES" if (report[1] & 0x04) else "NO")
		print("  Fault          :", faults[fault_code])
		print("  Count Remaining:", remaining)
		print("  Address        :", "0x{:02x}".format(report[4]))
		print("  Read           : [%s]" % (
			", ".join("0x{:02x}"
				.format(report[5 + i]) for i in range(0, num_bytes_read))))

	finally:
		dev.close()
