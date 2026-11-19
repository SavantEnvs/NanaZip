#!/usr/bin/env bash
#
# mayhem/build.sh — build NanaZip's own archive handlers (NanaZip.Codecs/NanaZip.Codecs.Archive.*.cpp)
# on Linux, one libFuzzer target per handler, plus a functional self-test.
#
# Upstream builds only with MSBuild on Windows. The handlers themselves are portable C++ written
# against the 7-Zip IInArchive interface; they need only a handful of Win32 types, which
# mayhem/shim/ provides (Windows.h + Mile.Helpers.CppBase.h stand-ins). Their NuGet dependencies
# (Mile.Mobility, Mile.Json) are fetched by the Dockerfile into /opt/toolchains/deps.
set -euo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${CXX:=clang++}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
: "${SRC:=/mayhem}"
: "${MILE_MOBILITY_DIR:=/opt/toolchains/deps/mile.mobility-1.1.732/src/Source/Native}"
: "${MILE_JSON_DIR:=/opt/toolchains/deps/mile.json-1.0.1057/src/Source/Native}"
cd "$SRC"

OUT="$SRC/mayhem/out"          # gitignored (mayhem/.gitignore)
mkdir -p "$OUT/fuzz" "$OUT/test"
CDX="$SRC/NanaZip.Codecs"
INC=(-I"$SRC/mayhem/shim" -I"$CDX" -I"$SRC/NanaZip.Specification" -I"$MILE_MOBILITY_DIR" -I"$MILE_JSON_DIR")
CXXSTD=(-std=c++20 -w -fms-extensions -fno-delayed-template-parsing)
HANDLERS=(Romfs Ufs DotNetSingleFile ElectronAsar Zealfs WebAssembly Littlefs)

# Common sources: the 7-Zip wrapper + helpers (project), the vendored littlefs (project), the shim.
PROJECT_C=("$CDX/LittleFS/lfs.c" "$CDX/LittleFS/lfs_util.c")
PROJECT_CXX=("$CDX/NanaZip.Codecs.SevenZipWrapper.cpp" "$CDX/Mile.Helpers.Portable.Base.Unstaged.cpp")
for h in "${HANDLERS[@]}"; do PROJECT_CXX+=("$CDX/NanaZip.Codecs.Archive.$h.cpp"); done
DEP_C=("$MILE_MOBILITY_DIR/Mile.Mobility.Utilities.FixedInteger.c" "$MILE_MOBILITY_DIR/Mile.Mobility.Utilities.MemoryAccess.c")
DEP_CXX=("$MILE_JSON_DIR/Mile.Json.cpp" "$SRC/mayhem/shim/shim.cpp")

# compile <objdir> <flags...> -- builds every source into <objdir>, prints nothing.
compile_set() {
  local od="$1" pflags="$2" dflags="$3"; mkdir -p "$od"
  local jobs=()
  for f in "${PROJECT_C[@]}"; do $CC $pflags "${INC[@]}" -c "$f" -o "$od/$(basename "$f").o" & jobs+=($!); done
  for f in "${PROJECT_CXX[@]}"; do $CXX $pflags "${CXXSTD[@]}" "${INC[@]}" -c "$f" -o "$od/$(basename "$f").o" & jobs+=($!); done
  for f in "${DEP_C[@]}"; do $CC $dflags "${INC[@]}" -c "$f" -o "$od/$(basename "$f").o" & jobs+=($!); done
  for f in "${DEP_CXX[@]}"; do $CXX $dflags "${CXXSTD[@]}" "${INC[@]}" -c "$f" -o "$od/$(basename "$f").o" & jobs+=($!); done
  for j in "${jobs[@]}"; do wait "$j"; done
  rm -f "$od/libnanazip_codecs.a"
  ar rcs "$od/libnanazip_codecs.a" "$od"/*.o
}

# 1) Fuzz build: the project sources (handlers, wrapper, littlefs) with sanitizers; deps uninstrumented.
# -fsanitize=fuzzer-no-link adds SanCov edge instrumentation to the project objects (the link-time
# $LIB_FUZZING_ENGINE alone instruments nothing that was compiled earlier).
FZ="$SANITIZER_FLAGS $DEBUG_FLAGS -O1"
FZCOV="$FZ -fsanitize=fuzzer-no-link"
compile_set "$OUT/obj-fuzz" "$FZCOV" "$DEBUG_FLAGS -O1"
$CXX $SANITIZER_FLAGS $DEBUG_FLAGS -c "$SRC/mayhem/lsan_off.cc" -o "$OUT/obj-fuzz/lsan_off.o"
$CC $SANITIZER_FLAGS $DEBUG_FLAGS -c "$STANDALONE_FUZZ_MAIN" -o "$OUT/obj-fuzz/standalone_main.o"

# 2) One harness per handler, each linked twice (libFuzzer + standalone reproducer).
for h in "${HANDLERS[@]}"; do
  $CXX $FZCOV "${CXXSTD[@]}" "${INC[@]}" -DCREATE="Create$h" -c "$SRC/mayhem/fuzz_archive.cpp" -o "$OUT/obj-fuzz/fuzz_$h.o"
  $CXX $FZ $LIB_FUZZING_ENGINE "$OUT/obj-fuzz/fuzz_$h.o" "$OUT/obj-fuzz/lsan_off.o" \
      "$OUT/obj-fuzz/libnanazip_codecs.a" -o "$OUT/fuzz/fuzz_$h"
  $CXX $FZ "$OUT/obj-fuzz/fuzz_$h.o" "$OUT/obj-fuzz/standalone_main.o" "$OUT/obj-fuzz/lsan_off.o" \
      "$OUT/obj-fuzz/libnanazip_codecs.a" -o "$OUT/fuzz/fuzz_$h-standalone"
done

# 3) Functional self-test with normal flags (no sanitizers): lists + extracts known archives.
compile_set "$OUT/obj-test" "-O2 $COVERAGE_FLAGS" "-O2"
$CXX -O2 $COVERAGE_FLAGS "${CXXSTD[@]}" "${INC[@]}" "$SRC/mayhem/nanazip_selftest.cpp" \
    "$OUT/obj-test/libnanazip_codecs.a" -o "$OUT/test/nanazip_selftest"
echo "build.sh: done"
