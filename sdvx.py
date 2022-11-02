from memory import MemorySearcher, match_phrase, decode_jisx0208
from time import sleep
import traceback


class SoundVoltexMemoryReader:
    values = {
        "artist": (b"info_usr/artist_selected_usr", 0xbd, "string"),
        "title": (b"info_usr/title_selected_usr", 0xbd, "string"),
        "illustrator": (b"info_usr/illust_usr", 0xbd, "string"),
        "effector": (b"info_usr/effect_usr", 0xc3, "string"),
        "clear_rate": (b"info_usr/rate_usr", 0xbb, "string"),
        "bpm": (b"info_usr/bpm_usr", 0xbb, "string"),
        "highscore": (b"rival_ranking_usr/1st_usr/score_usr", 0xbb, "string"),
        "category1": (b"category_01_usr/text_usr", 0xb1, "string"),
        "category2": (b"category_02_usr/text_usr", 0xb1, "string"),
    }
    matchings = {address_data[0]: name for name, address_data in values.items()}

    def __init__(self):
        self.memory_searcher = MemorySearcher("sv6c.exe")
        if self.memory_searcher.process is None:
            raise Exception("Could not find Sound Voltex process")

        self.running = False
        self.base_addresses = {name: None for name in self.values.keys()}
        self.data = {key: None for key in self.values.keys()}
        self.pages_read = 0

    def handle_page(self, page_info):
        self.pages_read += 1
        buf = self.memory_searcher.read_page(page_info)
        for match, name in self.matchings.items():
            matches = match_phrase(buf.raw, match)
            if matches:
                self.base_addresses[name] = page_info.BaseAddress + matches[0]
            if all(self.base_addresses.values()):
                return False
        return True

    def init(self):
        print("Opening process...")
        self.memory_searcher.open_process()
        print("Searching for base address...")
        self.memory_searcher.traverse_pages(self.handle_page)
        print("Running!")

    def run(self):
        self.running = True
        while self.running:
            for attribute in self.data.keys():
                try:
                    self.data[attribute] = self.get_from_memory(attribute)
                except:
                    print("Failed to read attribute "+attribute)
                    traceback.print_exc()
            sleep(1)

    def get_from_memory(self, attribute):
        if self.base_addresses[attribute] is None:
            return
        value = self.memory_searcher.read_address(self.base_addresses[attribute]+self.values[attribute][1], 100)
        if value is None:
            return
        value_type = self.values[attribute][2]
        if value_type == "string":
            if value.raw.startswith(b"\x00"):
                return ""
            data = value.raw.strip(b"\x00")
            if b"\x00" in data:
                data = data[:data.index(b"\x00")]
            return decode_jisx0208(data)
