#
# Hardware:
# Just the breakout board.
#
# Purpose:
# Obtain the Device Status Report, showing the reason for the last reset and
# any debugging information.
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

def get_device_status_report(dev):
	report_id = 0x02
	return dev.get_input_report(report_id, 41)

def array_str(arr, sep=", ", prefix="0x"):
	return sep.join("{}{:02x}".format(prefix, x) for x in arr)

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

		report = get_device_status_report(dev)
		print("\nReport: [%s]" % array_str(report))
		print("  Reset Reason                    : 0x%s" % array_str(report[4:0:-1], "", ""))
		print("  ERROREPC                        : 0x%s" % array_str(report[8:4:-1], "", ""))
		print("  EPC                             : 0x%s" % array_str(report[12:8:-1], "", ""))
		print("  CAUSE                           : 0x%s" % array_str(report[16:12:-1], "", ""))
		print("  DESAVE                          : 0x%s" % array_str(report[20:16:-1], "", ""))
		print("  RA                              : 0x%s" % array_str(report[24:20:-1], "", ""))
		print("  Debug DWORD                     : 0x%s" % array_str(report[32:24:-1], "", ""))
		print("  Configuration CRC32 (Stored)    : 0x%s" % array_str(report[36:32:-1], "", ""))
		print("  Configuration CRC32 (Calculated): 0x%s" % array_str(report[40:36:-1], "", ""))

	finally:
		dev.close()
