import secrets
import json
import struct
from .enums import CommandEnum, ClientState, Opcode, EventType, Result
from .events import EVENTS_MAP
from .exceptions import ClientConnectionException, DiscordException
from .winapi import open_file, read_all_file_contents, write_file, read_file
from .constants import IPC_PATH


class Payload:
    __slots__ = ("opcode", "command", "data", "nonce", "event")

    def __init__(self, opcode, data):
        self.opcode = opcode
        self.command = CommandEnum(data["cmd"])
        self.nonce = data["nonce"]
        self.event = EVENTS_MAP[data["evt"]](data["data"])

    @staticmethod
    def create_sending_payload(command, args):
        return {
            "cmd": command.value,
            "args": args,
            "nonce": secrets.token_urlsafe()
        }


class Client:
    def __init__(self, client_id):
        self.client_id = client_id
        self.handle = None
        self.state = ClientState.CONNECTING
        self._callbacks = {}

    # Commands

    def set_activity(self, pid, activity, callback=None):
        payload = Payload.create_sending_payload(CommandEnum.SET_ACTIVITY, {
            "pid": pid,
            "activity": activity.json
        })
        if callback is not None:
            self._register_callback(payload["nonce"], callback)
        self._send(Opcode.FRAME, payload)

    def connect(self, callback=None):
        self._open()
        if callback is not None:
            self._callbacks["READY"] = callback
        self._handshake()

    # Fundamental functionality

    def update(self):
        if self.state == ClientState.DISCONNECTED:
            raise ClientConnectionException("Client was disconnected.")

        payload = self._read()
        if payload is None:
            return
        if payload.opcode == Opcode.FRAME:
            if payload.event.type == EventType.READY:
                self.state = ClientState.CONNECTED
                if "READY" in self._callbacks:
                    self._callbacks["READY"]()
            elif payload.event.type == EventType.ERROR and payload.nonce in self._callbacks:
                self._callbacks[payload.nonce](Result.ERROR, payload.event)
            elif payload.event.type == EventType.ERROR:
                raise DiscordException(str(payload.event))
            elif payload.event.type == EventType.NONE and payload.nonce in self._callbacks:
                self._callbacks[payload.nonce](Result.OK, payload.event)
        elif payload.opcode == Opcode.CLOSE:
            self.state = ClientState.DISCONNECTED

    def _register_callback(self, nonce, callback):
        self._callbacks[nonce] = callback

    def _open(self):
        self.handle = open_file(IPC_PATH)

    def _read(self):
        bytes_read, raw = read_all_file_contents(self.handle)
        if bytes_read == 0:
            return
        opcode, length, _ = struct.unpack_from("BLh", raw, 0)
        payload = json.loads(raw[8:8+length].decode())
        print(f"Received payload ({opcode}): {payload}")
        return Payload(opcode, payload)

    def _send(self, opcode, payload):
        print(f"Sending payload ({opcode}): {payload}")
        payload_bytes = json.dumps(payload).encode()
        header = struct.pack("BL", opcode.value, len(payload_bytes))
        write_file(self.handle, header+payload_bytes)

    def _handshake(self):
        self._send(Opcode.HANDSHAKE, self.handshake_payload)

    @property
    def handshake_payload(self):
        return {
            "v": 1,
            "client_id": str(self.client_id)
        }
