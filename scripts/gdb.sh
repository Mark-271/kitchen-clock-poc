#!/bin/sh

# Author: Sam Protsenko <joe.skb7@gmail.com>

# Script for starting GDB server and GDB client
# [1] http://openocd.org/doc/html/GDB-and-OpenOCD.html

CROSS_COMPILE=arm-none-eabi-
GDB=${CROSS_COMPILE}gdb
ocd_iface="interface/stlink-v1.cfg"
ocd_target="target/stm32f1x.cfg"
# Don't execute "reset", as we want to debug running firmware, not load new one
# Only do "init" and "halt", to initialize JTAG and interrupt CPU execution
ocd_cmd="gdb_port pipe; log_output openocd.log; init; halt"
ocd_args="-f $ocd_iface -f $ocd_target -c \"$ocd_cmd\""
# Run OpenOCD from GDB via pipe so that GDB can start/stop OpenOCD for the
# debug session and can handle Ctrl-C correctly
gdb_args="target remote | openocd $ocd_args"

print_usage() {
	echo "Usage: $0 [--ddd | --tui] <elf_file>"
}

if [ "$1" = "--ddd" ]; then
	wrapper="ddd --debugger"
	shift
elif [ "$1" = "--tui" -o "$1" = "-tui" ]; then
	args="-tui"
	shift
fi

if [ $# -lt 1 ]; then
	echo "Error: Please provide elf-file for debug symbols" >&2
	print_usage $*
	exit 1
fi

elf_file="$1"
if [ ! -f "$elf_file" ]; then
	echo "Error: $elf_file doesn't exist" >&2
	print_usage $*
	exit 1
fi

killall openocd 2>/dev/null
$wrapper $GDB $args --eval-command="$gdb_args" $elf_file
