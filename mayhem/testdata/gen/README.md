How the seed / self-test archives in `mayhem/testdata/` were made:

- `a.romfs`, `a.asar`, `a.wasm`: hand-built minimal images (romfs `-rom1fs-` header + one file; asar
  16-byte pickle header + JSON index + data; wasm magic + type/function/code sections).
- `a.zealfs`, `a.dotnet`: `python3 gen.py <dir>` (ZealFS v1 image; .NET single-file bundle v6).
- `a.littlefs`: `genlfs.c` linked against NanaZip.Codecs/LittleFS (littlefs v2, 32 x 512-byte blocks).
- `a.ufs`: Debian `makefs -t ffs -o version=1 -s 256k` over a tree with `hello.txt`, `sub/big.dat`
  (20000 x 'U') and a symlink `link -> hello.txt`.
