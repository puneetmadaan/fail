#!/bin/bash
set -e
TARGET=experimentInfo.hpp

[ ! -e "$1" -o ! -e "$2" -o ! -e "$3" ] && echo "usage: $0 vanilla.elf guarded.elf plausibility.elf" && exit 1

function addrof() { nm -C $1 | (fgrep "$2" || echo 99999999) | awk '{print $1}'; }

cat >$TARGET <<EOF
#ifndef __WEATHERMONITOR_EXPERIMENT_INFO_HPP__
#define __WEATHERMONITOR_EXPERIMENT_INFO_HPP__

// autogenerated, don't edit!

// 0 = vanilla, 1 = guarded, 2 = plausibility
#define WEATHERMONITOR_VARIANT 0

#if WEATHERMONITOR_VARIANT == 0 // without vptr guards

EOF

function alldefs() {
cat <<EOF
// suffix for simulator state, trace file
#define WEATHER_SUFFIX				".`basename $1|sed s/\\\\..*$//`"
// main() address:
// nm -C $(basename $1)|fgrep main
#define WEATHER_FUNC_MAIN			0x`addrof $1 main`
// wait_begin address
#define WEATHER_FUNC_WAIT_BEGIN		0x`addrof $1 wait_begin`
// wait_end address
#define WEATHER_FUNC_WAIT_END		0x`addrof $1 wait_end`
// vptr_panic address (only exists in guarded variant)
#define WEATHER_FUNC_VPTR_PANIC		0x`addrof $1 vptr_panic`
// number of main loop iterations to trace
// (determines trace length and therefore fault-space width)
#define WEATHER_NUMITER_TRACING		4
// number of instructions needed for these iterations in golden run (taken from
// experiment step #2)
#define WEATHER_NUMINSTR_TRACING	20599
// number of additional loop iterations for FI experiments (to see whether
// everything continues working fine)
#define WEATHER_NUMITER_AFTER		2
// number of instructions needed for these iterations in golden run (taken from
// experiment step #2)
#define WEATHER_NUMINSTR_AFTER		10272
// data/BSS begin:
// nm -C $(basename $1)|fgrep ___DATA_START__
#define WEATHER_DATA_START			0x`addrof $1 ___DATA_START__`
// data/BSS end:
// nm -C $(basename $1)|fgrep ___BSS_END__
#define WEATHER_DATA_END			0x`addrof $1 ___BSS_END__`
// text begin:
// nm -C $(basename $1)|fgrep ___TEXT_START__
#define WEATHER_TEXT_START			0x`addrof $1 ___TEXT_START__`
// text end:
// nm -C $(basename $1)|fgrep ___TEXT_END__
#define WEATHER_TEXT_END			0x`addrof $1 ___TEXT_END__`
EOF
}

alldefs $1 >>$TARGET
cat >>$TARGET <<EOF

#elif WEATHERMONITOR_VARIANT == 1 // with guards

EOF
alldefs $2 >>$TARGET
cat >>$TARGET <<EOF

#elif WEATHERMONITOR_VARIANT == 2 // with guards + plausibility check

EOF
alldefs $3 >>$TARGET
cat >>$TARGET <<EOF

#else
#error Unknown WEATHERMONITOR_VARIANT
#endif

#endif
EOF
