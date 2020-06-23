import time

class SampledPlayer:
	def __init__(self, player, interval):
		self._player = player
		self._interval = interval
		self._next_advance = 0

	def advance(self, ball):
		now = time.perf_counter()
		if now >= self._next_advance:
			self._next_advance = now + self._interval
			self._player.advance(ball)

	def reset(self, position):
		self._player.reset(position)

	def draw(self, display):
		self._player.draw(display)

	@property
	def x(self):
		return self._player.x

	@property
	def y(self):
		return self._player.y
