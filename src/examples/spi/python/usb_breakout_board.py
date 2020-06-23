import hid

class UsbBreakoutBoard:
	def __init__(self, vid, pid):
		self._vid = vid
		self._pid = pid
		self._core_intf = None
		self._spi_intf = None

	def __enter__(self):
		self._core_intf = UsbBreakoutBoardInterface(self._vid, self._pid, 0)
		self._spi_intf = UsbBreakoutBoardInterface(self._vid, self._pid, 6)
		return self

	def __exit__(self, *args):
		if self._core_intf is not None:
			self._core_intf.__exit__()

		if self._spi_intf is not None:
			self._spi_intf.__exit__()

	@property
	def core(self):
		return self._core_intf

	@property
	def spi(self):
		return self._spi_intf

class UsbBreakoutBoardInterface:
	def __init__(self, vid, pid, intf_index):
		self._vid = vid
		self._pid = pid
		self._id = intf_index
		self.__dev = None

	@property
	def id(self):
		return self._id

	def __enter__(self):
		return self

	def __exit__(self, *args):
		if self.__dev is not None:
			self.__dev.close()

	def read_report(self):
		report = []
		while True:
			report_part = self._dev.read(64, timeout_ms=100)
			if report_part is None:
				continue

			if len(report_part) == 0:
				if len(report) == 0:
					continue
				else:
					break

			report += report_part
			if len(report_part) < 64:
				break

		return report

	@property
	def _dev(self):
		if self.__dev is None:
			self._lazy_init()

		return self.__dev

	def _lazy_init(self):
		path = self._get_path_to_device_interface()
		if path != "":
			self.__dev = hid.device()
			self.__dev.open_path(path)

		if self.__dev is None:
			raise Exception(
				"USB Type-C / PIC32 Breakout Board is not connected; interface={}, path={}."
				.format(self._id, path))

	def _get_path_to_device_interface(self):
		for ls in hid.enumerate():
			if ls["vendor_id"] == self._vid and ls["product_id"] == self._pid and ls["interface_number"] == self._id:
				return ls["path"]

		return ""

	def write_report(self, report, expected_ack=0x01):
		self._dev.write(report)
		if expected_ack is None:
			return

		while True:
			ack = self.read_report()
			if ack[0] == 0x01 and ack[1] == report[0]:
				if ack[2] != 0:
					raise Exception("Got NACK for report ID 0x{:02x}, with status 0x{:02x}".format(ack[1], ack[2]))
				else:
					return ack
			elif ack[0] == expected_ack:
				return ack

	def print_details(self):
		print("Manufacturer :", self._dev.get_manufacturer_string())
		print("Product      :", self._dev.get_product_string())
		print("Serial Number:", self._dev.get_serial_number_string())
