#
# Hardware:
# Breakout board with pin A3 connected to a 330ohm resistor which is in turn
# connected to an LED to ground.  And the same for the C9 pin.
#
# Purpose:
# Lights A3 if the port is connected via the CC1 pin (facing up), or C9 if
# the port is connected via the CC2 pin (facing down).
#

import hid

VID = 0x1209
PID = 0xcb0b
CORE_INTERFACE = 0x00

def get_path_to_device_interface(intf):
	for ls in hid.enumerate():
		if ls["vendor_id"] == VID and ls["product_id"] == PID and ls["interface_number"] == intf:
			return ls["path"]

	return ""

def send_report(dev, report):
	dev.write(report)
	ack = dev.read(7)
	if ack[0] != 0x01 or ack[1] != report[0]:
		raise Exception("Unexpected report ID 0x{:02x} / 0x{:02x} when waiting for ACK".format(ack[0], ack[1]))

	if ack[2] != 0:
		raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))

	return ack[2:]


def configure_pin_as_flag_output(dev, pin_id, flags_mask):
	report_id = 0x40 | pin_id
	unlock_key = [0x00] * 8
	suspend_behaviour = 0x00
	usb_interface_id = 0x03
	cable_flags_function_id = 0x08
	pin_behaviour = 0x08

	send_report(
		dev,
		[report_id] + unlock_key + [suspend_behaviour, usb_interface_id,
		cable_flags_function_id, pin_behaviour] + flags_mask)

def configure_a3_as_cc1(dev):
	configure_pin_as_flag_output(dev, 0x03, [0x40, 0x00, 0x00, 0x00, 0x00, 0x00])

def configure_c9_as_cc2(dev):
	configure_pin_as_flag_output(dev, 0x29, [0x80, 0x00, 0x00, 0x00, 0x00, 0x00])

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

		configure_a3_as_cc1(dev)
		configure_c9_as_cc2(dev)

	finally:
		dev.close()
