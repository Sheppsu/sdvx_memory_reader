from time import time


class Config:
    __slots__ = ("cdn_host", "api_endpoint", "environment")

    def __init__(self, data):
        self.cdn_host = data["cdn_host"]
        self.api_endpoint = data["api_endpoint"]
        self.environment = data["environment"]


class User:
    __slots__ = (
        "id", "username", "discriminator", "avatar", "bot", "system",
        "mfa_enabled", "banner", "accent_color", "locale", "verified",
        "email", "flags", "premium_type", "public_flags"
    )

    def __init__(self, data):
        self.id = data["id"]
        self.username = data["username"]
        self.discriminator = data["discriminator"]
        self.avatar = data["avatar"]

        self.bot = data.get("bot")
        self.system = data.get("system")
        self.mfa_enabled = data.get("mfa_enabled")
        self.banner = data.get("banner")
        self.accent_color = data.get("accent_color")
        self.locale = data.get("locale")
        self.verified = data.get("verified")
        self.email = data.get("email")
        self.flags = data.get("flags")
        self.premium_type = data.get("premium_type")
        self.public_flags = data.get("public_flags")


class BasePayloadObject:
    __slots__ = ()

    @property
    def json(self):
        payload = {}
        for attr in self.__slots__:
            value = getattr(self, attr)
            if value is None:
                continue
            if isinstance(value, BasePayloadObject):
                value = value.json
            payload[attr] = value
        return payload


class ActivityTimestamps(BasePayloadObject):
    __slots__ = ("start", "end")

    def __init__(self, start=None, end=None):
        self.start = start
        self.end = end

    @classmethod
    def now(cls):
        return cls(int(time()))


class ActivityEmoji(BasePayloadObject):
    __slots__ = ("name", "emoji_id", "animation")

    def __init__(self, name, emoji_id=None, animation=None):
        self.name = name
        self.emoji_id = emoji_id
        self.animation = animation


class ActivityPart(BasePayloadObject):
    __slots__ = ("id", "size")

    def __init__(self, party_id, size):
        self.id = party_id
        self.size = size


class ActivityAssets(BasePayloadObject):
    __slots__ = ("large_image", "large_text", "small_image", "small_text")

    def __init__(self, large_image=None, large_text=None, small_image=None,
                 small_text=None):
        self.large_image = large_image
        self.large_text = large_text
        self.small_image = small_image
        self.small_text = small_text


class ActivitySecrets(BasePayloadObject):
    __slots__ = ("join", "spectate", "match")

    def __init__(self, join=None, spectate=None, match=None):
        self.join = join
        self.spectate = spectate
        self.match = match


class Activity(BasePayloadObject):
    __slots__ = (
        "created_at", "timestamps", "application_id", "details", "state", "emoji",
        "party", "assets", "secrets", "instance", "flags", "buttons"
    )

    def __init__(self, created_at=None, timestamps=None, application_id=None, details=None,
                 state=None, emoji=None, party=None, assets=None, secrets=None, instance=None,
                 flags=None, buttons=None):
        self.created_at = created_at
        self.timestamps = timestamps
        self.application_id = application_id
        self.details = details
        self.state = state
        self.emoji = emoji
        self.party = party
        self.assets = assets
        self.secrets = secrets
        self.instance = instance
        self.flags = flags  # TODO: flags enum
        self.buttons = buttons  # TODO: button object
