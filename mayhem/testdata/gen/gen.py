#!/usr/bin/env python3
import struct, sys
out = sys.argv[1]
# ZealFS v1: 8 pages of 256 bytes. page0 = header(64) + 6 root entries(32 each).
P = 256
img = bytearray(8 * P)
hdr = bytes([ord('Z'), 1, 1, 8 - 5]) + bytes([0x1f]) + bytes(31) + bytes(28)
img[0:64] = hdr
def ent(flags, name, start, size):
    return struct.pack('<B16sBH', flags, name.encode(), start, size) + bytes([0x20, 0x24, 0x05, 0x17, 0x04, 0x12, 0x30, 0x45]) + bytes(4)
img[64:96] = ent(0x80, 'hello.txt', 1, 300)
img[96:128] = ent(0x81, 'sub', 3, 0)
data = bytes((i * 13 + 1) & 0xff for i in range(300))
img[P:P+1] = b'\x02'; img[P+1:P+256] = data[:255]
img[2*P:2*P+1] = b'\x00'; img[2*P+1:2*P+46] = data[255:]
img[3*P:3*P+32] = ent(0x80, 'a.bin', 4, 10)
img[4*P:4*P+11] = b'\x00' + b'ZealFS!!\n\x00'
open(out + '/a.zealfs', 'wb').write(img)
# .NET single-file bundle v6
sig = bytes([0x8b,0x12,0x02,0xb9,0x6a,0x61,0x20,0x38,0x72,0x7b,0x93,0x02,0x14,0xd7,0xa0,0x32,0x13,0xf5,0xb9,0xe6,0xef,0xae,0x33,0x18,0xee,0x3b,0x2d,0xce,0x24,0xb3,0x6a,0xae])
pre = b'MZ' + bytes(14)
f1 = b'{"runtimeOptions":{}}'; f2 = bytes(range(64))
off1 = len(pre) + 8 + 32; off2 = off1 + len(f1); hoff = off2 + len(f2)
def e(o, s, t, p):
    return struct.pack('<qqqB', o, s, 0, t) + bytes([len(p)]) + p.encode()
h = struct.pack('<III', 6, 0, 2) + bytes([4]) + b'abcd' + struct.pack('<qqqqQ', 0, 0, off1, len(f1), 0)
h += e(off1, len(f1), 4, 'app.runtimeconfig.json') + e(off2, len(f2), 1, 'lib/App.dll')
open(out + '/a.dotnet', 'wb').write(pre + struct.pack('<q', hoff) + sig + f1 + f2 + h)
