#
# Hardware:
# Breakout board with pin A3 connected to a 330ohm resistor which is in turn
# connected to an LED to ground.  And the same for the C9 pin.
#
# Purpose:
# Illustrates the masked-set functionality of the GPIO interface by configuring
# the two LEDs as the two low-order bits and incrementing a counter to toggle
# them.
#

import time
import hid

VID = 0x1209
PID = 0xcb0b
CORE_INTERFACE = 0x00

def get_path_to_device_interface(intf):
	for ls in hid.enumerate():
		if ls["vendor_id"] == VID and ls["product_id"] == PID and ls["interface_number"] == intf:
			return ls["path"]

	return ""

def send_report(dev, report, expected_ack=0x01):
	dev.write(report)
	while True:
		ack = dev.read(16)
		if ack[0] == 0x01 and ack[1] == report[0]:
			if ack[2] != 0:
				raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))
			else:
				return ack
		elif ack[0] == expected_ack:
			return ack

def configure_pin_as_output(dev, pin_id, pin_index):
	report_id = 0x40 | pin_id
	unlock_key = [0x00] * 8
	suspend_behaviour = 0x00
	core_interface_id = 0x00
	output_function_id = 0x01
	pin_behaviour = 0x00
	interrupt_behaviour = 0x00
	reserved = [0x00] * 4

	send_report(
		dev,
		[report_id] + unlock_key + [suspend_behaviour, core_interface_id,
		output_function_id, pin_behaviour, interrupt_behaviour, pin_index] +
		reserved)

def configure_a3_as_led_output1(dev):
	configure_pin_as_output(dev, 0x03, 0)

def configure_c9_as_led_output2(dev):
	configure_pin_as_output(dev, 0x29, 1)

def mask_to_leds(dev, mask):
	report_id = 0x14
	load = 0x03
	send_report(dev, [report_id, load, mask & 0xff, (mask >> 8) & 0xff], expected_ack=0x16)

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

		configure_a3_as_led_output1(dev)
		configure_c9_as_led_output2(dev)
		count = 0
		while True:
			time.sleep(0.5)
			count = (count + 1) & 0x03
			mask_to_leds(dev, count)

	finally:
		dev.close()
