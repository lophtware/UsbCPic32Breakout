class SpiBus:
	def __init__(self, core_intf, spi_intf, clk_pin, mosi_pin, miso_pin, slaves):
		self._core_intf = core_intf
		self._spi_intf = spi_intf
		self._configure_clock_as(clk_pin)
		self._configure_mosi_as(mosi_pin)
		self._configure_miso_as(miso_pin)
		for slave in slaves:
			self._configure_slave_select_as(slave.address, slave.select_pin, slave.is_select_pin_active_high)
			self._configure_slave(slave)

	def _configure_clock_as(self, pin):
		function_id = 0x01
		low_no_pulls = 0x00
		reserved = [0x00] * 6
		return self._configure_spi_pin(pin, [function_id, low_no_pulls] + reserved)

	def _configure_spi_pin(self, pin, function):
		report_id = 0x40 | pin
		unlock_key = [0x00] * 8
		suspend_behaviour = 0x00
		spi_interface_id = self._spi_intf.id
		return self._core_intf.write_report(
			[report_id] +
			unlock_key +
			[suspend_behaviour, spi_interface_id] +
			function)

	def _configure_mosi_as(self, pin):
		function_id = 0x03
		low_no_pulls = 0x00
		reserved = [0x00] * 6
		return self._configure_spi_pin(pin, [function_id, low_no_pulls] + reserved)

	def _configure_miso_as(self, pin):
		function_id = 0x02
		low_no_pulls = 0x00
		reserved = [0x00] * 6
		return self._configure_spi_pin(pin, [function_id, low_no_pulls] + reserved)

	def _configure_slave_select_as(self, address, pin, is_active_high):
		function_id = 0x08 | (address & 0x07)
		inactive_no_pulls = 0x00 | (0x08 if not is_active_high else 0x00)
		reserved = [0x00] * 6
		return self._configure_spi_pin(pin, [function_id, inactive_no_pulls] + reserved)

	def _configure_slave(self, slave):
		report_id = 0x10 | ((slave.address & 0x07) << 1)
		unlock_key = [0x00] * 8

		UPDATE_IMMEDIATE = 0xc0

		baud_rate_flags = UPDATE_IMMEDIATE
		baud_rate = int(24 / slave.baud_rate_mhz / 2 - 1)

		clock_flags = UPDATE_IMMEDIATE
		clock_flags |= 0x02 if slave.clock_idles_high else 0
		clock_flags |= 0x01 if slave.mosi_changes_on_active_to_idle_edge else 0

		others_unchanged = [0x00] * 5

		return self._spi_intf.write_report(
			[report_id] +
			unlock_key + [
				baud_rate_flags, baud_rate & 0xff, (baud_rate >> 8) & 0xff,
				clock_flags] +
			others_unchanged)

	def send_spi_transaction_report(self, report, show_progress=False):
		bytes = []
		self._spi_intf.write_report(report, expected_ack=None)
		while True:
			ack = self._spi_intf.read_report()

			if ack[0] == 0x01 and ack[1] == report[0]:
				if ack[2] != 0:
					raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))
				else:
					return bytes

			elif ack[0] & 0x80:
				timeout = ack[1] & 0x01
				done = ack[1] & 0x02
				in_count = (ack[0] & 0x7f) * 8 + ((ack[1] >> 5) & 7)
				in_remaining = (ack[3] << 8) | ack[2]
				if timeout:
					raise Exception("SPI transaction timed out; header=0x{:02x}{:02x}{:02x}{:02x}".format(ack[0], ack[1], ack[2], ack[3]))

				if show_progress:
					print("Received {} bytes, {} remaining...".format(in_count, in_remaining))

				bytes += ack[5:5 + in_count]
				if done:
					return bytes

class SpiSlave:
	def __init__(self, address, ss_pin, ss_is_active_high=False, baud_rate_mhz=1, clock_idles_high=True, mosi_changes_on_active_to_idle_edge=False):
		self._address = address
		self._select_pin = ss_pin
		self._is_select_pin_active_high = ss_is_active_high
		self._baud_rate_mhz = baud_rate_mhz
		self._clock_idles_high = clock_idles_high
		self._mosi_changes_on_active_to_idle_edge = mosi_changes_on_active_to_idle_edge

	@property
	def address(self):
		return self._address

	@property
	def select_pin(self):
		return self._select_pin

	@property
	def is_select_pin_active_high(self):
		return self._is_select_pin_active_high

	@property
	def baud_rate_mhz(self):
		return self._baud_rate_mhz

	@property
	def clock_idles_high(self):
		return self._clock_idles_high

	@property
	def mosi_changes_on_active_to_idle_edge(self):
		return self._mosi_changes_on_active_to_idle_edge

	def send_transaction(self, bus, out, in_count, show_progress=False):
		residual = len(out) & 7
		out_report = [
			0x80 + (len(out) // 8),
			residual << 5,
			in_count & 0xff,
			(in_count >> 8) & 0xff,
			self.address] + out + [0x00] * (7 - residual)

		return bus.send_spi_transaction_report(out_report, show_progress)
