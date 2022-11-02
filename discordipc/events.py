from .objects import *
from .enums import EventType


class UnnamedEvent:
    type = EventType.NONE

    def __init__(self, data):
        for key, value in data.items():
            if key == "type":
                key = "event_type"
            setattr(self, key, value)


class ReadyEvent:
    type = EventType.READY
    __slots__ = ("version", "config", "user")

    def __init__(self, data):
        self.version = data["v"]
        self.config = Config(data["config"])
        self.user = User(data["user"])


class ErrorEvent:
    type = EventType.ERROR
    __slots__ = ("code", "message")

    def __init__(self, data):
        self.code = data["code"]
        self.message = data["message"]

    def __repr__(self):
        return f"Error {self.code}: {self.message}"


EVENTS_MAP = {
    None: UnnamedEvent,
    "READY": ReadyEvent,
    "ERROR": ErrorEvent,
}
