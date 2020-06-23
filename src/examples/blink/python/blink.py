#
# Hardware:
# Breakout board with pin A3 connected to a 330ohm resistor which is in turn
# connected to an LED to ground.
#
# Purpose:
# A 'Hello, World !' example - blinking LED using the GPIO interface.
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

def configure_a3_as_led_output(dev):
	report_id = 0x43
	unlock_key = [0x00] * 8
	suspend_behaviour = 0x00
	core_interface_id = 0x00
	output_function_id = 0x01
	pin_behaviour = 0x00
	interrupt_behaviour = 0x00
	led_pin_id = 0x00
	reserved = [0x00] * 4

	send_report(
		dev,
		[report_id] + unlock_key + [suspend_behaviour, core_interface_id,
		output_function_id, pin_behaviour, interrupt_behaviour, led_pin_id] +
		reserved)

	return led_pin_id

def toggle_pin(dev, pin_id):
	report_id = 0x14
	toggle = 0x02
	mask = 1 << pin_id
	send_report(dev, [report_id, toggle, mask & 0xff, (mask >> 8) & 0xff], expected_ack=0x16)

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

		led = configure_a3_as_led_output(dev)
		while True:
			time.sleep(1)
			toggle_pin(dev, led)

	finally:
		dev.close()
