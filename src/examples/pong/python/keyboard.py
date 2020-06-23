import threading

class KeyboardThread(threading.Thread):
	def __init__(self, on_input, name = 'keyboard-input-thread'):
		self._on_input = on_input
		super(KeyboardThread, self).__init__(name=name)
		self.setDaemon(True)
		self.start()

	def run(self):
		while not self._on_input(input()):
			True
