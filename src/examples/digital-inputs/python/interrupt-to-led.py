#
# Hardware:
# Breakout board with pin A3 connected to a 330ohm resistor which is in turn
# connected to an LED to ground.  C9 should be connected to ground, +3.3V or
# some other logic-level signal (or switch).
#
# Purpose:
# The C9 pin is set to interrupt on change, which will issue a Pins Changed
# report that is used to set the value of the A3 pin (LED).
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

def send_report_and_wait_for_ack(dev, report):
	dev.write(report)
	ack = dev.read(16)
	if ack[0] == 0x16:
		return ack[2:]
	elif ack[0] != 0x01 or ack[1] != report[0]:
		raise Exception("Unexpected report ID 0x{:02x} / 0x{:02x} when waiting for ACK".format(ack[0], ack[1]))

	if ack[2] != 0:
		raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))

	return ack[2:]

def configure_pin_as_input_with_continuous_change_notification(dev, pin_id, pin_index):
	as_input = 0x10
	pull_down = 0x01
	pull_up = 0x02

	report_id = 0x40 | pin_id
	unlock_key = [0x00] * 8
	suspend_behaviour = 0x00
	core_interface_id = 0x00
	output_function_id = 0x00
	pin_behaviour = as_input | pull_down
	interrupt_behaviour = 0x07
	reserved = [0x00] * 4

	send_report_and_wait_for_ack(
		dev,
		[report_id] + unlock_key + [suspend_behaviour, core_interface_id,
		output_function_id, pin_behaviour, interrupt_behaviour, pin_index] +
		reserved)

def configure_pin_as_output(dev, pin_id, pin_index):
	report_id = 0x40 | pin_id
	unlock_key = [0x00] * 8
	suspend_behaviour = 0x00
	core_interface_id = 0x00
	output_function_id = 0x01
	pin_behaviour = 0x00
	interrupt_behaviour = 0x00
	reserved = [0x00] * 4

	send_report_and_wait_for_ack(
		dev,
		[report_id] + unlock_key + [suspend_behaviour, core_interface_id,
		output_function_id, pin_behaviour, interrupt_behaviour, pin_index] +
		reserved)

def configure_a3_as_led_output(dev):
	configure_pin_as_output(dev, 0x03, 0)
	return 0

def configure_c9_as_led_input(dev):
	configure_pin_as_input_with_continuous_change_notification(dev, 0x29, 1)
	return 1

def read_from_change_notification_report(dev, pin_index):
	print("Waiting for notification of Pins Changed...", end='', flush=True)

	report = dev.read(16, timeout_ms=3 * 1000)
	if len(report) == 0:
		print("timed out.")
		return False

	if report[0] != 0x18:
		raise Exception("Unexpected report whilst waiting for Pins Changed; id=" + report[0])

	print("received !  [%s]" % ", ".join("0x{:02x}".format(x) for x in report))
	changed = (report[5] << 8) | report[4]
	return True if changed & (1 << pin_index) else False

def set_led_status(dev, pin_index, is_on):
	report_id = 0x14
	load = 0x03
	mask = (1 << pin_index) if is_on else 0
	send_report_and_wait_for_ack(dev, [report_id, load, mask & 0xff, (mask >> 8) & 0xff])

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

		led = configure_a3_as_led_output(dev)
		input_pin = configure_c9_as_led_input(dev)
		while True:
			is_on = read_from_change_notification_report(dev, input_pin)
			set_led_status(dev, led, is_on)

	finally:
		dev.close()
