from enum import Enum, IntEnum


class CommandEnum(Enum):
    DISPATCH = "DISPATCH"
    AUTHORIZE = "AUTHORIZE"
    AUTHENTICATE = "AUTHENTICATE"
    GET_GUILD = "GET_GUILD"
    GET_GUILDS = "GET_GUILDS"
    GET_CHANNEL = "GET_CHANNELS"
    SUBSCRIBE = "SUBSCRIBE"
    UNSUBSCRIBE = "UNSUBSCRIBE"
    SET_USER_VOICE_SETTINGS = "SET_USER_VOICE_SETTINGS"
    SELECT_VOICE_CHANNEL = "SELECT_VOICE_CHANNEL"
    GET_SELECTED_VOICE_CHANNEL = "GET_SELECTED_VOICE_CHANNEL"
    SELECT_TEXT_CHANNEL = "SELECT_TEXT_CHANNEL"
    GET_VOICE_SETTINGS = "GET_VOICE_SETTINGS"
    SET_VOICE_SETTINGS = "SET_VOICE_SETTINGS"
    SET_CERTIFIED_DEVICES = "SET_CERTIFIED_DEVICES"
    SET_ACTIVITY = "SET_ACTIVITY"
    SEND_ACTIVITY_JOIN_INVITE = "SEND_ACTIVITY_JOIN_INVITE"
    CLOSE_ACTIVITY_REQUEST = "CLOSE_ACTIVITY_REQUEST"


class EventType(Enum):
    NONE = None
    READY = "READY"
    ERROR = "ERROR"


class ClientState(IntEnum):
    CONNECTING = 0
    CONNECTED = 1
    DISCONNECTED = 2


class Opcode(IntEnum):
    HANDSHAKE = 0
    FRAME = 1
    CLOSE = 2
    PING = 3
    PONG = 4


class ActivityType(IntEnum):
    GAME = 0
    STREAMING = 1
    LISTENING = 2
    WATCHING = 3
    CUSTOM = 4
    COMPETING = 5


class Result(IntEnum):
    OK = 0
    ERROR = 1