# SPDX-License-Identifier: GPL-3.0-only

.section .rodata
.global g_devices
.global g_devices_size

.align 4
g_devices:
.incbin "../src/devices.dat"

.align 4
g_devices_size:
.int g_devices_size - g_devices
