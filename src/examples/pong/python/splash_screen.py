import time

class SplashScreen:
	def __init__(self, tile):
		self._tile = tile

	def show(self, display, timeout_s = 3):
		centred_x = (display.width - self._tile.width) // 2
		centred_y = (display.height - self._tile.height) // 2
		display.set_tile(centred_x, centred_y, self._tile)
		display.set_brightness(0)
		display.blit()

		brightness_increment = 1
		for brightness in range(brightness_increment, 100 + brightness_increment, brightness_increment):
			time.sleep(2 / (100 / brightness_increment))
			display.set_brightness(brightness / 100)

		time.sleep(timeout_s)
