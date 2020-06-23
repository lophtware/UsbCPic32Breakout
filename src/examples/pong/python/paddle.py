import math

from box import Box

class Paddle:
	_MAX_BOUNCE_ANGLE = math.radians(75)

	def __init__(self, position, velocity, bounding_box, tile):
		self._position = position
		self._velocity = velocity
		self._bounding_box = bounding_box
		self._tile = tile

		self._half_width = self._tile.width / 2
		self._half_height = self._tile.height / 2

		self._display_position = [int(position[0]), int(position[1])]
		self._previous_position = None

	def _clip(self, position):
		if self._bounding_box.contains(Box(position[0], position[1], self._tile.width, self._tile.height)):
			return [position, self._velocity]

		y = position[1]
		y_velocity = self._velocity
		if position[1] < self._bounding_box.y:
			y = self._bounding_box.y
			y_velocity = 0
		elif position[1] + self._tile.height >= self._bounding_box.max_y:
			y = self._bounding_box.max_y - self._tile.height
			y_velocity = 0

		return [[self._position[0], y], y_velocity]

	def _return_angle_from_quadrant(self, ball_dx, ball_dy):
		normalised_dy = ball_dy / self._half_height
		if ball_dx > 0:
			return -normalised_dy * Paddle._MAX_BOUNCE_ANGLE
		else:
			return math.pi + normalised_dy * Paddle._MAX_BOUNCE_ANGLE

	def advance(self, ball):
		[self._position, self._velocity] = self._clip([self._position[0], self._position[1] + self._velocity])

		paddle_x = self._position[0] + self._half_width
		paddle_y = self._position[1] + self._half_height

		ball_dx = ball.x - paddle_x
		ball_dy = ball.y - paddle_y

		if (abs(ball_dx) <= ball.radius + self._half_width) and (abs(ball_dy) <= ball.radius + self._half_height):
			return_angle = self._return_angle_from_quadrant(ball_dx, ball_dy)
			ball.set_direction(return_angle)

	def draw(self, display):
		if self._previous_position is not None:
			display.xor_tile(self._previous_position[0], self._previous_position[1], self._tile)

		[x, y] = [int(self._position[0]), int(self._position[1])]
		display.xor_tile(x, y, self._tile)
		self._previous_position = [x, y]

	def set_position(self, y):
		self._position[1] = y - self._half_height

	def move(self, velocity):
		self._velocity = velocity

	def as_box(self):
		return Box(self._position[0], self._position[1], self._tile.width, self._tile.height)

	@property
	def x(self):
		return self._position[0] + self._half_width

	@property
	def y(self):
		return self._position[1] + self._half_height
