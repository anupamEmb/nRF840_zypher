# CLAUDE.md — AI Assistant Guide for nRF840_zypher

This document provides guidance for AI assistants (Claude and others) working on the `nRF840_zypher` project. It covers the project's purpose, expected structure, development workflows, and conventions to follow.

---

## Project Overview

**nRF840_zypher** is an RTOS-based firmware project targeting the **Nordic Semiconductor nRF52840** microcontroller, developed using the **Zephyr RTOS** and the **nRF Connect SDK (NCS)** toolchain.

- **MCU**: Nordic nRF52840 (ARM Cortex-M4F, 64 MHz, 1 MB Flash, 256 KB RAM)
- **RTOS**: Zephyr RTOS (via nRF Connect SDK)
- **Build System**: CMake + west (Zephyr's meta-tool)
- **Language**: C (primary), with optional C++ or assembly
- **License**: GNU General Public License v3.0

---

## Repository Layout (Expected Structure)

This is a greenfield project. As it grows, the following canonical Zephyr application structure should be maintained:

```
nRF840_zypher/
├── CLAUDE.md               # This file
├── README.md               # Human-facing project overview
├── LICENSE                 # GPLv3
├── CMakeLists.txt          # Top-level CMake build file
├── prj.conf                # Zephyr Kconfig project defaults
├── Kconfig                 # Application-level Kconfig definitions
├── west.yml                # West manifest (Zephyr + NCS dependencies)
│
├── src/                    # Application source code
│   ├── main.c              # Entry point (main() or k_thread entry)
│   └── <module>/           # Feature modules (one subdirectory per concern)
│       ├── <module>.c
│       ├── <module>.h
│       └── CMakeLists.txt
│
├── include/                # Public headers (shared across modules)
│   └── <module>.h
│
├── boards/                 # Board-specific overlays (if custom board)
│   └── nrf52840dk_nrf52840.overlay
│
├── dts/                    # Device Tree Source fragments/overlays
│   └── *.overlay
│
├── drivers/                # Custom out-of-tree Zephyr drivers (if any)
│
├── lib/                    # Reusable libraries decoupled from the app
│
├── tests/                  # Zephyr twister/unit tests
│   └── <test_suite>/
│       ├── src/main.c
│       ├── CMakeLists.txt
│       └── prj.conf
│
├── scripts/                # Helper scripts (flashing, log parsing, etc.)
│
└── .github/                # CI/CD (GitHub Actions workflows)
    └── workflows/
        └── build.yml
```

---

## Build System

### Prerequisites

Install the nRF Connect SDK toolchain via **nRF Connect for Desktop** or the VS Code extension. Key tools:

| Tool | Purpose |
|------|---------|
| `west` | Zephyr's meta-tool for build, flash, and manifest management |
| `cmake` (≥ 3.20) | Build configuration |
| `ninja` | Preferred build backend |
| `nrfjprog` / `JLinkExe` | Flashing via J-Link |
| `nRF Connect SDK` | Zephyr fork + Nordic drivers and libraries |

### Workspace Initialization

```bash
# Initialize west workspace (run once, outside the repo)
west init -m https://github.com/<org>/nRF840_zypher --mr main workspace/
cd workspace/
west update

# Or if the west manifest is in this repo:
west init .
west update
```

### Building

```bash
# Build for nRF52840 DK (default dev board)
west build -b nrf52840dk/nrf52840 app/

# Clean build
west build -b nrf52840dk/nrf52840 app/ --pristine

# Build with extra Kconfig overrides
west build -b nrf52840dk/nrf52840 app/ -- -DCONFIG_LOG=y -DCONFIG_LOG_LEVEL_DBG=y
```

### Flashing

```bash
# Flash via J-Link
west flash

# Flash a specific hex file
west flash --hex-file build/zephyr/zephyr.hex

# Erase and flash
west flash --erase
```

### Debugging

```bash
# Open GDB debug session
west debug

# RTT logging (Nordic's Real-Time Transfer)
JLinkRTTViewer
# or
nrfjprog --rtt
```

---

## Key Configuration Files

### `prj.conf`

The primary Kconfig configuration for the application. Defines enabled subsystems, logging levels, stack sizes, and feature flags. Example:

```kconfig
# Kernel
CONFIG_MAIN_STACK_SIZE=4096
CONFIG_HEAP_MEM_POOL_SIZE=8192

# Logging
CONFIG_LOG=y
CONFIG_LOG_BACKEND_UART=y
CONFIG_LOG_DEFAULT_LEVEL=3

# Bluetooth (if used)
CONFIG_BT=y
CONFIG_BT_PERIPHERAL=y

# Shell (optional debugging aid)
CONFIG_SHELL=y
```

### `CMakeLists.txt`

Must begin with the Zephyr boilerplate:

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(nRF840_zypher)

target_sources(app PRIVATE src/main.c)
```

### `west.yml`

Declares the NCS version and any additional repositories:

```yaml
manifest:
  remotes:
    - name: ncs
      url-base: https://github.com/nrfconnect
  projects:
    - name: sdk-nrf
      remote: ncs
      revision: v2.7.0
      import: true
  self:
    path: nRF840_zypher
```

---

## Coding Conventions

### C Code Style

- Follow **Zephyr's coding style**: based on Linux kernel style with 8-space tabs.
- Use `clang-format` with Zephyr's `.clang-format` config if available.
- File names: `snake_case.c`, `snake_case.h`
- Macros: `ALL_CAPS_WITH_UNDERSCORES`
- Functions: `module_verb_noun()` pattern (e.g., `ble_adv_start()`)
- Structs: `struct snake_case_name`
- Kconfig symbols: `CONFIG_MODULE_FEATURE` (always prefixed with `CONFIG_`)

### Header Guards

Use `#pragma once` or traditional include guards:

```c
#ifndef MY_MODULE_H_
#define MY_MODULE_H_

/* ... */

#endif /* MY_MODULE_H_ */
```

### Zephyr-Specific Patterns

- **Threads**: Declare with `K_THREAD_DEFINE()` macro or `k_thread_create()`.
- **Semaphores/Mutexes**: Use `K_SEM_DEFINE()`, `K_MUTEX_DEFINE()`.
- **Work Queues**: Prefer `k_work` / `k_work_delayable` over raw threads for deferred work.
- **Logging**: Use `LOG_MODULE_REGISTER(module_name, LOG_LEVEL_DBG)` per file; use `LOG_INF()`, `LOG_WRN()`, `LOG_ERR()`, `LOG_DBG()`.
- **Device Tree**: Access peripherals via DT macros (`DT_NODELABEL()`, `DEVICE_DT_GET()`); never hardcode peripheral addresses.
- **Error handling**: Check return values of all Zephyr API calls; use `__ASSERT()` for invariants during development.

### Example `main.c` Pattern

```c
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
    LOG_INF("nRF840_zypher starting");

    /* Initialize modules */

    /* Application loop or thread delegation */
    while (1) {
        k_sleep(K_FOREVER);
    }

    return 0;
}
```

---

## Testing

### Zephyr Twister (Integration / On-Target Tests)

```bash
# Run all tests
west twister -T tests/ -p nrf52840dk/nrf52840

# Run with verbose output
west twister -T tests/ -p nrf52840dk/nrf52840 -v

# Native POSIX target (unit tests without hardware)
west twister -T tests/ -p native_posix
```

### Unit Test Structure

Each test suite lives in `tests/<suite_name>/` and must contain:
- `src/main.c` — test cases using `ztest` framework
- `CMakeLists.txt` — minimal, sourcing `ztest`
- `prj.conf` — test-specific Kconfig

```c
#include <zephyr/ztest.h>

ZTEST(suite_name, test_something)
{
    zassert_equal(1 + 1, 2, "Basic arithmetic failed");
}

ZTEST_SUITE(suite_name, NULL, NULL, NULL, NULL, NULL);
```

---

## Development Workflow

### Branch Strategy

- `master` — stable, tagged releases only
- `develop` — integration branch for feature PRs
- `feature/<description>` — individual feature branches
- `fix/<description>` — bug fix branches
- `claude/<session-id>` — AI-assisted development branches (auto-created)

### Commit Messages

Follow **Conventional Commits**:

```
<type>(<scope>): <short summary>

[optional body]

[optional footer]
```

Types: `feat`, `fix`, `refactor`, `docs`, `test`, `chore`, `ci`

Examples:
```
feat(ble): add BLE advertisement module
fix(uart): correct baud rate initialization for UART1
docs(readme): update build instructions for NCS v2.7
test(sensor): add unit tests for temperature conversion
```

### Pull Request Checklist

- [ ] Code compiles without warnings (`west build`)
- [ ] Relevant tests added or updated
- [ ] `prj.conf` changes documented with reason
- [ ] Device Tree changes include comments explaining the hardware mapping
- [ ] Logging added at appropriate levels (no debug prints via `printk` in production code)
- [ ] Stack/heap sizing reviewed if new threads added

---

## Common West Commands Reference

```bash
# Update all west dependencies
west update

# List all boards supported by Zephyr
west boards

# Check build for multiple boards
west build -b nrf52840dk/nrf52840 && west build -b nrf52840dongle/nrf52840

# Generate compile_commands.json for IDE/clangd
west build -b nrf52840dk/nrf52840 -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cp build/compile_commands.json .

# Menuconfig (interactive Kconfig browser)
west build -t menuconfig

# Guiconfig (GUI Kconfig browser)
west build -t guiconfig
```

---

## Hardware Targets

| Board ID | Hardware | Notes |
|----------|----------|-------|
| `nrf52840dk/nrf52840` | nRF52840 Development Kit | Primary dev board |
| `nrf52840dongle/nrf52840` | nRF52840 USB Dongle | For USB HID/CDC testing |
| `native_posix` | Linux process simulation | Unit tests without hardware |

---

## AI Assistant Guidelines

When working on this codebase:

1. **Read before modifying** — Always read existing files before editing. Understand existing patterns before introducing new ones.
2. **Zephyr APIs only** — Never use POSIX/stdlib functions that are unavailable in Zephyr's minimal libc (e.g., `malloc` should be replaced with `k_malloc` or memory pools).
3. **Device Tree first** — Hardware configuration belongs in `.overlay` / `.dts` files, not `#define`s in C code.
4. **Kconfig for features** — New optional features must be gated by a `CONFIG_` symbol, never by hardcoded `#ifdef`.
5. **No bare metal register access** — Use Zephyr HAL/driver APIs; do not write directly to nRF52840 peripheral registers unless writing a custom driver.
6. **Check Zephyr version compatibility** — API changes between NCS versions are common; verify the target NCS version in `west.yml` before using newer APIs.
7. **Stack overflow awareness** — Zephyr stacks are fixed-size; when adding functionality to threads, consider increasing `CONFIG_*_STACK_SIZE` in `prj.conf`.
8. **Minimize `printk` usage** — Use the Zephyr logging subsystem (`LOG_*` macros) for all diagnostic output so logging can be disabled in production builds.
9. **Test on `native_posix` first** — Pure logic can be tested on `native_posix` without hardware, which is faster for iteration.
10. **Respect GPL v3** — All source files added to this project must be compatible with GPL v3. Do not incorporate code under incompatible licenses.

---

## Useful Resources

- [Zephyr Project Documentation](https://docs.zephyrproject.org/)
- [nRF Connect SDK Documentation](https://developer.nordicsemi.com/nRF_Connect_SDK/doc/latest/)
- [nRF52840 Product Specification](https://infocenter.nordicsemi.com/topic/struct_nrf52/struct/nrf52840.html)
- [West Documentation](https://docs.zephyrproject.org/latest/develop/west/index.html)
- [Zephyr Kconfig Reference](https://docs.zephyrproject.org/latest/kconfig.html)
- [Zephyr Device Tree Guide](https://docs.zephyrproject.org/latest/build/dts/index.html)
- [nRF52840 DK User Guide](https://infocenter.nordicsemi.com/topic/ug_nrf52840_dk/UG/dk/intro.html)
