from sdvx import SoundVoltexMemoryReader
from discordipc import Client, ClientState, Activity, ActivityTimestamps, ActivityAssets
import threading


mem_reader = SoundVoltexMemoryReader()
mem_reader.init()
mem_run = threading.Thread(target=mem_reader.run)
mem_run.start()


def run():
    client = Client(1032756213445836801)
    client.connect(lambda: print("Running!"))
    last_artist = None
    while client.state == ClientState.CONNECTING or client.state == ClientState.CONNECTED:
        client.update()
        if client.state == ClientState.CONNECTED and mem_reader.data["artist"] is not None and \
                mem_reader.data["artist"] != last_artist:
            last_artist = mem_reader.data["artist"]
            assets = ActivityAssets("sdvx")
            details = f"{mem_reader.data['artist']} - {mem_reader.data['title']}"
            activity = Activity(timestamps=ActivityTimestamps.now(),
                                details=details, assets=assets)
            client.set_activity(mem_reader.memory_searcher.process.pid, activity,
                                lambda result, event: print(f"set_activity result: {str(result)}"))

try:
    run()
except KeyboardInterrupt:
    mem_reader.running = False
