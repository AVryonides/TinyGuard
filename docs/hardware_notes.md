# TinyGuard Hardware Notes

## Board

- Board/module: ESP32-S3-WROOM-1
- Port: /dev/cu.usbmodem11201
- ESP-IDF version: v6.0.1
- Chip model: ESP32-S3
- Chip revision: v0.2
- CPU cores: 2
- Physical flash: 16 MB
- Configured flash: 16 MB
- PSRAM: 8 MB embedded PSRAM, detected and initialized
- PSRAM mode: Octal
- PSRAM speed: 80 MHz

## Initial Configuration Issue

The first hardware probe run reported only 2 MB configured flash even though esptool detected 16 MB physical flash. It also reported PSRAM as not detected or not initialized even though esptool reported 8 MB embedded PSRAM.

## Configuration Fix

The ESP-IDF project configuration was updated to:

- `CONFIG_ESPTOOLPY_FLASHSIZE_16MB=y`
- `CONFIG_ESPTOOLPY_FLASHSIZE="16MB"`
- `CONFIG_SPIRAM=y`
- `CONFIG_SPIRAM_MODE_OCT=y`
- `CONFIG_SPIRAM_SPEED_80M=y`
- `CONFIG_SPIRAM_USE_MALLOC=y`

## Before vs After

| Metric | Before | After |
|---|---:|---:|
| Configured flash | 2 MB | 16 MB |
| Physical flash detected | 16 MB | 16 MB |
| PSRAM initialized | No | Yes |
| Free heap | 400,988 bytes | 8,748,316 bytes |
| Internal 8-bit free heap | 400,988 bytes | 394,039 bytes |
| Internal DMA-capable heap | 393,200 bytes | 386,251 bytes |
| Largest internal block | 335,872 bytes | 294,912 bytes |
| Free PSRAM | 0 bytes | 8,386,308 bytes |
| Largest PSRAM block | 0 bytes | 8,257,536 bytes |

## Interpretation

The ESP32-S3 board physically includes 16 MB flash and 8 MB embedded PSRAM. The initial ESP-IDF project configuration exposed only 2 MB flash and did not initialize PSRAM. After updating the flash and SPIRAM configuration, the firmware now correctly detects 16 MB flash and initializes 8 MB PSRAM.

This matters for TinyGuard because future TinyML workloads may require larger model binaries, tensor arenas, communication buffers, or backend/hybrid inference support. Internal SRAM is still limited, so performance-critical buffers should remain internal when possible, while larger less latency-sensitive allocations can use PSRAM.

## Latest Hardware Probe Output

```text
============================================================
 TinyGuard Hardware Probe
============================================================
ESP-IDF version: v6.0.1
Chip model: ESP32-S3
Chip revision: v0.2
CPU cores: 2
Features: Wi-Fi BLE

Memory information
------------------
Free heap: 8748316 bytes
Minimum free heap since boot: 8748316 bytes
Internal 8-bit capable free heap: 394039 bytes
Internal DMA-capable free heap: 386251 bytes
Largest free internal block: 294912 bytes
PSRAM: detected and available
Free PSRAM: 8386308 bytes
Largest free PSRAM block: 8257536 bytes

Flash information
-----------------
Flash size: 16777216 bytes (16.00 MB)

Runtime information
-------------------
Reset reason: USB peripheral reset
Time since boot: 106552 us

Hardware probe complete.
Next step: save this output into docs/hardware_notes.md
============================================================