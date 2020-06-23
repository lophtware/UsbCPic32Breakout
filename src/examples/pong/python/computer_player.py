import random

class ComputerPlayer:
	def __init__(self, paddle, max_speed, difficulty):
		self._paddle = paddle
		self._max_speed = max_speed
		self._difficulty = difficulty

	def advance(self, ball):
		if abs(ball.x - self._paddle.x) <= 40 and random.random() < self._difficulty:
			if ball.y < self._paddle.y - 3:
				self._paddle.move(-self._max_speed)
			elif ball.y > self._paddle.y + 3:
				self._paddle.move(self._max_speed)
			else:
				self._paddle.move(0)

		self._paddle.advance(ball)

	def reset(self, position):
		self._paddle.move(0)
		self._paddle.set_position(position)

	def draw(self, display):
		self._paddle.draw(display)

	@property
	def x(self):
		return self._paddle.x

	@property
	def y(self):
		return self._paddle.y
