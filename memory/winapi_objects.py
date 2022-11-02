import ctypes as ct
from ctypes.wintypes import *


SIZE_T = ct.c_size_t
PVOID = LPVOID


def structure_to_string_method(self):
    ret = [f"{self.__class__.__name__} (size: {ct.sizeof(self.__class__)}) instance at 0x{id(self):016X}:"]
    for fn, _ in self._fields_:
        ret.append(f"  {fn}: {getattr(self, fn)}")
    return "\n".join(ret) + "\n"


class Struct(ct.Structure):
    to_string = structure_to_string_method


class MEMORY_BASIC_INFORMATION(Struct):
    _fields_ = (
        ("BaseAddress", PVOID),
        ("AllocationBase", PVOID),
        ("AllocationProtect", DWORD),
        ("PartitionId", WORD),
        ("RegionSize", SIZE_T),
        ("State", DWORD),
        ("Protect", DWORD),
        ("Type", DWORD),
    )