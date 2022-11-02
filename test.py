from sdvx import SoundVoltexMemoryReader
from time import sleep
import threading


mem_reader = SoundVoltexMemoryReader()
mem_reader.init()
run = threading.Thread(target=mem_reader.run)
run.start()

try:
    while True:
        if mem_reader.data["artist"] is not None:
            print(mem_reader.data)
        sleep(3)
except KeyboardInterrupt:
    mem_reader.running = False
