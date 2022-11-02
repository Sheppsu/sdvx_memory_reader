import ctypes as ct
from ctypes.wintypes import *
import psutil
from .winapi_objects import MEMORY_BASIC_INFORMATION, SIZE_T
from .constants import *


k32 = ct.WinDLL('kernel32')
OpenProcess = k32.OpenProcess
OpenProcess.argtypes = DWORD, BOOL, DWORD
OpenProcess.restype = HANDLE
ReadProcessMemory = k32.ReadProcessMemory
ReadProcessMemory.argtypes = HANDLE, LPVOID, LPVOID, ct.c_size_t, ct.POINTER(ct.c_size_t)
ReadProcessMemory.restype = BOOL
VirtualQueryEx = k32.VirtualQueryEx
VirtualQueryEx.argtypes = HANDLE, LPCVOID, ct.POINTER(MEMORY_BASIC_INFORMATION), SIZE_T
VirtualQueryEx.restype = SIZE_T


def requires_handle(error_msg):
    def wrapper(func):
        def check(self, *args, **kwargs):
            if self.proch is None:
                raise Exception(error_msg)
            return func(self, *args, **kwargs)
        return check
    return wrapper


class MemorySearcher:
    def __init__(self, process_name):
        self.process = self.find_process(process_name)
        if self.process is None:
            print("Process not found")
            return

        self.proch = None

    @staticmethod
    def find_process(name):
        name = name.lower()
        for proc in psutil.process_iter():
            try:
                proc_name = proc.name()
                if proc_name.lower() == name:
                    return proc
            except (psutil.NoSuchProcess, psutil.AccessDenied, psutil.ZombieProcess):
                pass

    def open_process(self, dw_desired_access=PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, b_inherit_handle=0, pid=None):
        if pid is None:
            if self.process is None:
                raise Exception("MemorySearcher does not have a set process (set it or provide a pid).")
            pid = self.process.pid
        self.proch = OpenProcess(dw_desired_access, b_inherit_handle, pid)
        if self.proch is None:
            raise Exception(f"Error opening process: Error code {k32.GetLastError()}")

    @requires_handle("iterate_pages cannot be used without a process handle (call open_process first)")
    def traverse_pages(self, callback):
        addr = 0
        while True:
            info = MEMORY_BASIC_INFORMATION()
            bytes_returned = VirtualQueryEx(self.proch, addr, ct.byref(info), ct.sizeof(info))
            addr += info.RegionSize
            if bytes_returned == 0:
                error = k32.GetLastError()
                if error == ERROR_INVALID_PARAMETER:
                    print("Reached end of pages.")
                else:
                    print(f"Error occurred: {error}")
                break
            elif not info.Protect & PAGE_NOACCESS and not info.Protect & PAGE_GUARD and info.Protect != 0:
                if not callback(info):
                    break

    def read_page(self, page_info):
        buf = self.read_address(page_info.BaseAddress, page_info.RegionSize)
        if buf is None:
            print("Info object:")
            for attr in MEMORY_BASIC_INFORMATION._fields_:
                print(f"{attr[0]} = {getattr(page_info, attr[0])!r}")
        else:
            return buf

    def read_address(self, address, size):
        buf = ct.create_string_buffer(size)
        size_buf = ct.c_size_t()
        if ReadProcessMemory(self.proch, address, buf, ct.sizeof(buf), ct.byref(size_buf)):
            return buf
        else:
            print(f"Failed to read {hex(address)}; Error code: {k32.GetLastError()}")
