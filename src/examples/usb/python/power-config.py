#
# Hardware:
# Just the breakout board.
#
# Purpose:
# Gets and sets the current limits for the current configuration.
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

def get_power_configuration(dev):
	report_id = 0x08
	return dev.get_input_report(report_id, 7)

def set_power_configuration(dev, enumerated_limit_ma, charger_limit_ma, charger_test):
	report_id = 0x08
	update_enumerated_limit_flag = 0x80
	update_charger_test_flag = 0x80 if charger_test is not None else 0x00
	update_charger_limit_flag = 0x40 if charger_limit_ma is not None else 0x00
	charger_test_flag = 0x01 if charger_test else 0x00
	charger_flags = update_charger_limit_flag | update_charger_test_flag | charger_test_flag

	charger_limit_ma = charger_limit_ma if charger_limit_ma is not None else 0

	return send_report_and_wait_for_ack(dev, [
		report_id,
		update_enumerated_limit_flag, enumerated_limit_ma & 0xff, (enumerated_limit_ma >> 8) & 0xff,
		charger_flags, charger_limit_ma & 0xff, (charger_limit_ma >> 8) & 0xff])

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

		if len(sys.argv) > 1:
			enumerated_limit_ma = int(sys.argv[1])
			charger_limit_ma = None if len(sys.argv) < 3 else int(sys.argv[2])
			charger_limit_test = None if len(sys.argv) < 4 else (sys.argv[3] == "enable-charger-test")
			print("Setting current limits...", end='', flush=True)
			report = set_power_configuration(dev, enumerated_limit_ma, charger_limit_ma, charger_limit_test)
			print("done")
		else:
			report = get_power_configuration(dev)
			print("Report: [%s]" % ", ".join("0x{:02x}".format(x) for x in report))
			print("\tEnumerated Current Limit                   : {}mA".format((report[3] << 8) | report[2]))
			print("\tAssumed Current Limit for Dedicated Charger: {}mA".format((report[6] << 8) | report[5]))
			print("\tDedicated Charger Test on Power-Up         :", "Enabled" if (report[4] & 0x01) else "Disabled")

	finally:
		dev.close()
