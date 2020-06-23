#
# Hardware:
# A USB C / PIC32 Breakout Board connected to an SSD1306-based OLED display
# (128x64 pixels) and an M5Stack joystick, both via I2C.
#
# Purpose:
# Illustrates the I2C Master functionality to manipulate two I2C Slaves for
# the purposes of the Kickstarter demo.
#

from usb_device import UsbDevice
from upside_down_display import UpsideDownDisplay
from ssd1306_i2c_slave_display import Ssd1306I2cSlaveDisplay
from keyboard import KeyboardThread
from splash_screen import SplashScreen
from font import Font

from box import Box
from tile import Tile
from ball import Ball
from paddle import Paddle
from playing_area import PlayingArea
from game_loop import GameLoop
from sampled_player import SampledPlayer
from computer_player import ComputerPlayer
from i2c_joystick_player import I2cJoystickPlayer

if __name__ == "__main__":
	with UsbDevice() as usb:
		usb.i2c.baud_rate_400khz()

		display = UpsideDownDisplay(Ssd1306I2cSlaveDisplay(usb.i2c.slave(0x3c)))
		display.initialise()

		with open("font-5x8.raw", "rb") as font_fd:
			font_5x8 = Font(Tile.from_raw(font_fd.read(), 32 * 6, 4 * 9), 6, 9)

		playing_area = PlayingArea(display)
		paddle_tile = Tile(2, 12, [0b11000000] * 12)
		paddle_x_offset = 2
		paddle_y_offset = 2
		paddle_speed = 1.5
		paddles = [
			Paddle(
				[playing_area.bounding_box.x + paddle_x_offset, -1],
				0,
				Box(
					playing_area.bounding_box.x + paddle_x_offset,
					playing_area.bounding_box.y + paddle_y_offset,
					paddle_tile.width,
					playing_area.bounding_box.height - 2 * paddle_y_offset),
				paddle_tile),
			Paddle(
				[playing_area.bounding_box.max_x - 1 - paddle_x_offset - paddle_tile.width // 2, -1],
				0,
				Box(
					playing_area.bounding_box.max_x - 1 - paddle_x_offset - paddle_tile.width,
					playing_area.bounding_box.y + paddle_y_offset,
					paddle_tile.width,
					playing_area.bounding_box.height - 2 * paddle_y_offset),
				paddle_tile)]

		players = [
			ComputerPlayer(paddles[0], max_speed = paddle_speed, difficulty = 0.1),
			SampledPlayer(
				I2cJoystickPlayer(paddles[1], usb.i2c.slave(0x52), 1, -128, paddle_speed / 128),
				interval = 0.01),
		]

		game = GameLoop(
			playing_area,
			Ball(
				playing_area.centre,
				[1.8, 0],
				playing_area.bounding_box,
				Tile(6, 6, [
					0b00110000,
					0b01111000,
					0b11111100,
					0b11111100,
					0b01111000,
					0b00110000])),
			players,
			score_font = font_5x8,
			message_font = font_5x8)

		with open("lophtware-128x64.raw", "rb") as logo_fd:
			logo = Tile.from_raw(logo_fd.read(), 128, 64)

		SplashScreen(logo).show(display)

		quit = False
		def on_keyboard_input(cmd):
			global quit
			quit = cmd == 'q'
			return quit

		keyboard = KeyboardThread(on_keyboard_input)

		playing_area.draw()
		while not quit:
			if not game.do_frame(display):
				break

			display.blit()

		with open("thanks-128x64.raw", "rb") as thanks_fd:
			thanks = Tile.from_raw(thanks_fd.read(), 128, 64)

		SplashScreen(thanks).show(display)
