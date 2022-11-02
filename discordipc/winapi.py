import ctypes as ct
from ctypes.wintypes import *
from .exceptions import WinapiException


__all__ = (
    "open_file", "read_file", "write_file", "read_all_file_contents",
)


# Winapi constants

GENERIC_READ = -0x80000000
GENERIC_WRITE = 0x40000000
OPEN_EXISTING = 0x3
INVALID_HANDLE_VALUE = -0x1
ERROR_IO_PENDING = 0x3E5


# Winapi structures

class SecurityAttributes(ct.Structure):
    _fields_ = (
        ("nLength", DWORD),
        ("lpSecurityDescriptor", LPVOID),
        ("bInheritHandle", BOOL)
    )


class OverlappedUnionStruct(ct.Structure):
    _fields_ = (
        ("Offset", DWORD),
        ("OffsetHigh", DWORD)
    )


class OverlappedUnion(ct.Union):
    _fields_ = (
        ("Offset", OverlappedUnionStruct),
        ("Pointer", LPVOID)
    )


class Overlapped(ct.Structure):
    _fields_ = (
        ("Internal", PULONG),
        ("InternalHigh", PULONG),
        ("Data", OverlappedUnion),
        ("hEvent", HANDLE)
    )


# Winapi functions

k32 = ct.WinDLL("kernel32")
GetLastError = k32.GetLastError
GetLastError.restype = DWORD
CreateFile = k32.CreateFileA
CreateFile.argtypes = LPCSTR, DWORD, DWORD, ct.POINTER(SecurityAttributes), DWORD, DWORD, HANDLE
CreateFile.restype = HANDLE
ReadFile = k32.ReadFile
ReadFile.argtypes = HANDLE, LPVOID, DWORD, LPDWORD, ct.POINTER(Overlapped)
ReadFile.restype = BOOL
WriteFile = k32.WriteFile
WriteFile.argtypes = HANDLE, LPCVOID, DWORD, LPDWORD, ct.POINTER(Overlapped)
WriteFile.restype = BOOL
PeekNamedPipe = k32.PeekNamedPipe
PeekNamedPipe.argtypes = HANDLE, LPVOID, DWORD, LPDWORD, LPDWORD, LPDWORD
PeekNamedPipe.restype = BOOL


# Wrapper functions


def _peek_pipe(handle):
    bytes_available = ct.c_ulong()
    if not PeekNamedPipe(handle, None, 0, None, ct.byref(bytes_available), None):
        raise WinapiException(f"PeekNamedPipe failed with error code {GetLastError()}")
    return bytes_available.value


def open_file(file_name, access=GENERIC_READ | GENERIC_WRITE, share_mode=0, security_attribute=None,
              creation_disposition=OPEN_EXISTING, flags=0, template_file=None):
    file_name = ct.c_char_p(file_name.encode())
    if security_attribute is None:
        security_attribute = SecurityAttributes()
    handle = CreateFile(file_name, access, share_mode, security_attribute, creation_disposition,
                        flags, template_file)
    if handle == INVALID_HANDLE_VALUE:
        raise WinapiException(f"CreateFile failed with error code {GetLastError()}")
    return handle


def read_file(handle, size):
    buf = ct.create_string_buffer(size)
    bytes_read = DWORD()
    if ReadFile(handle, ct.byref(buf), size, ct.byref(bytes_read), None):
        data = buf.raw[:bytes_read.value]
        return bytes_read.value, data
    elif error_code := GetLastError() != ERROR_IO_PENDING:
        raise WinapiException(f"ReadFile failed with error code {error_code}")


def read_all_file_contents(handle):
    data = bytes()
    total_bytes_read = 0
    while bytes_available := _peek_pipe(handle):
        result = read_file(handle, bytes_available)
        if result is None:
            break
        total_bytes_read += result[0]
        data += result[1]
    return total_bytes_read, data


def write_file(handle, data):
    bytes_written = ct.c_ulong()
    if WriteFile(handle, data, len(data), ct.byref(bytes_written), Overlapped()):
        return bytes_written.value
    else:
        raise WinapiException(f"WriteFile failed with error code {GetLastError()}")
