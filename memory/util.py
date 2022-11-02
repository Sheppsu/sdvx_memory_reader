import numpy as np
import os


basedir = os.path.dirname(os.path.abspath(__file__))
jis_path = os.path.join(basedir, "JIS0208.TXT")
if not os.path.exists(jis_path):
    jis_path = "JIS0208.TXT"

jis = np.zeros((6879,), np.int32)
unicode = np.zeros((6879,), np.int32)
try:
    f = open(jis_path, "r")
except FileNotFoundError:
    raise FileNotFoundError("JIS0208.TXT file must be in the same directory as the exe file.")
while (line := f.readline().strip()).startswith("#"): pass
for i in range(jis.shape[0]):
    h1, _, h2, _ = line.split("\t")
    jis[i], unicode[i] = int(h1[2:], 16), int(h2[2:], 16)
    line = f.readline().strip()
f.close()


def match_phrase(buf, phrase):
    matches = []
    while phrase in buf:
        matches.append(buf.index(phrase))
        buf = buf[matches[-1]+len(phrase):]
    return matches


def decode_jisx0208(byte_str):
    byte_arr = bytearray(byte_str)
    out = ""
    i = 0
    while i < len(byte_arr):
        if exists := np.where(jis == int.from_bytes(byte_arr[i:i+2], "big"))[0]:
            out += chr(unicode[exists[0]])
            i += 2
        else:
            out += chr(byte_arr[i])
            i += 1
    return out

