import time

class UsbI2cSlave:
	def __init__(self, dev, bus_address):
		self._dev = dev
		self._bus_address = bus_address

	def _read_in_chunks(self, count):
		out = []
		while True:
			report = []
			while len(report) == 0 or (report[0] & 0x80) == 0:
				report = self._dev.read(128)

			if report[4] != self._bus_address:
				raise Exception(
					"Corrupted I2C transaction ?  Expected address 0x{:02x} but got 0x{:02x}"
					.format(self._bus_address, report[4]))

			if (report[1] & 0x07) != 0:
				raise Exception("I2C Error; flags=0x{:02x}".format(report[1] & 0x1f))

			chunk_size = (report[0] & 0x7f) * 8 + ((report[1] >> 5) & 0x07)
			out += report[5:6 + chunk_size]
			count = (report[3] << 8) | report[2]
			if count == 0:
				break

		return out

	def write_then_read(self, payload_out, count_in):
		report_id = 0x80 + int(len(payload_out) / 8)
		residual = len(payload_out) % 8
		flags = residual << 5
		count_low = (count_in >> 0) & 0xff
		count_high = (count_in >> 8) & 0xff
		report = (
			[report_id, flags, count_low, count_high, self._bus_address] +
			payload_out + [0x00] * (7 - residual))

		print("WRITE REPORT: [%s]" % ", ".join("0x{:02x}".format(x) for x in report))
		self._dev.write(report)
		return self._read_in_chunks(count_in)

	def write(self, payload):
		self.write_then_read(payload, 0)

	def read(self, count):
		return self.write_then_read([], count)

class UsbI2cInterface:
	_BAUD_RATE_400KHZ = 29

	def __init__(self, dev):
		self._dev = dev

	def slave(self, bus_address):
		return UsbI2cSlave(self._dev, bus_address)

	def _send_report_wait_for_ack(self, report):
		self._dev.write(report)
		ack = self._dev.read(7)
		if ack[0] != 0x01 or ack[1] != report[0]:
			raise Exception("Unexpected report ID 0x{:02x} / 0x{:02x} when waiting for ACK".format(ack[0], ack[1]))

		if ack[2] != 0:
			raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))

		return ack[2:]

	def baud_rate_400khz(self):
		report_id = 0x06
		unlock_key = [0x00] * 8
		baud_low = (UsbI2cInterface._BAUD_RATE_400KHZ >> 0) & 0xff
		baud_high = (UsbI2cInterface._BAUD_RATE_400KHZ >> 8) & 0xff
		ignored = [0x00] * 15
		self._send_report_wait_for_ack([report_id] + unlock_key + [0xc0, baud_low, baud_high] + ignored)
