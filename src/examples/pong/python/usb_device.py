import hid
from usb_i2c_interface import UsbI2cInterface

class UsbDevice:
	_I2C_INTERFACE_ID = 0x04

	def __init__(self, vid = 0x1209, pid = 0xcb0b):
		self._vid = vid
		self._pid = pid
		self.i2c = None

	def _get_path_to_device_interface(self, intf):
		for ls in hid.enumerate():
			if (
				ls["vendor_id"] == self._vid and
				ls["product_id"] == self._pid and
				ls["interface_number"] == intf):
				return ls["path"]

		return ""

	def __enter__(self):
		i2c_path = self._get_path_to_device_interface(UsbDevice._I2C_INTERFACE_ID)
		if not i2c_path:
			raise Exception("USB device is not attached")

		self._i2c_dev = hid.device()
		self._i2c_dev.open_path(i2c_path)
		self.i2c = UsbI2cInterface(self._i2c_dev)
		return self

	def __exit__(self, *args):
		if self._i2c_dev is not None:
			self._i2c_dev.close()
