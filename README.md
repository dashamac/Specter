# FUSE Driver

A Windows kernel mode driver implementing physical memory manipulation, CR3 (page table root) management, and user mode communication capabilities.

## Project Structure

```
fuse driver/
├── driver/                          # Kernel mode driver
│   ├── main.cpp                    # Driver entry point
│   └── project/
│       ├── core/
│       │   └── status_codes.hpp    # Status codes
│       ├── communication/          # Communication system
│       ├── cr3 decryption/         # CR3 decryption
│       ├── interrupts/             # Interrupt handling
│       ├── logging/                # Logging system
│       ├── physmem/                # Physical memory management
│       └── ...
│
├── mapper/                          # User mode application
│   ├── main.cpp
│   ├── intel_driver.cpp
│   ├── kdmapper.cpp                # Kernel mapper
│   └── includes/
│
└── api/                             # Shared headers
    ├── driver/
    └── proc/
```

## Main Components

### Kernel Driver (roseware_driver)

Kernel component implements:

- **Physical Memory Management**: Direct physical memory manipulation via page tables
- **IPC Communication**: Kernel to user mode communication channel
- **CR3 Manipulation**: CR3 encryption/decryption and caching
- **Interrupt Handling**: NMI handler installation and management
- **Structured Logging**: Debug logging system

### Mapper (kdmapper)

User mode application that:

- Loads driver into kernel using mapping techniques
- Communicates with driver for operations
- Manages offsets and debug symbols

## Build Requirements

- Visual Studio 2022 Community or higher
- Windows Driver Kit (WDK) 10.0.22621.0 or higher
- Spectre-mitigated libraries installed
- C++20 support

## Building

### Setup WDK

1. Open Visual Studio Installer
2. Modify Visual Studio Community installation
3. Check "Desktop development with C++"
4. In dependencies, check "Windows SDK" (latest version)
5. Install "Spectre-mitigated libraries" from Individual Components

### Compile

```bash
# Open the solution
physmem.sln

# Build using Visual Studio
# Release|x64 is the recommended configuration
```

## Code Architecture

### Status Codes

All driver returns use the `project_status` enum defined in `project/core/status_codes.hpp`. Error codes are organized by category:

- `0x0000xxxx` - Generic errors
- `0x0001xxxx` - CR3 errors
- `0x0002xxxx` - Memory copy errors
- `0x0003xxxx` - Process errors
- `0x0004xxxx` - Module errors
- `0x0005xxxx` - Paging errors

### Communication Structure

Kernel to user mode communication happens through shared buffers defined in `communication/shared_structs.hpp`.

### CR3 Cache

The driver maintains a CR3 cache to optimize repeated operations. Cache is managed by timestamp with configurable TTL.

## Modules

### physmem
Direct physical memory manipulation:
- Read/write physical addresses
- Page table management
- Memory remapping

### communication
IPC between kernel and user mode:
- Command handlers
- Checksum validation
- Operation timeouts

### interrupts
System interrupt management:
- NMI handler installation
- IDT manipulation

### cr3 decryption
CR3 manipulation functions:
- CR3 value decryption
- Per-process CR3 caching

### logging
Structured logging system with severity levels.

## Build Output

The project builds for x64 only. Release configuration is optimized for performance.

Generated files:
- `x64/Release/roseware_driver.sys` - Kernel driver
- `x64/Release/kdmapper.exe` - Mapping application

## Security Notes

This is a WDM driver with full kernel access. Use only in development and test environments.

Features requiring kernel mode:
- Page table manipulation
- Interrupt handler installation
- Direct physical memory access

## Troubleshooting

**Error: "Spectre-mitigated libraries are required"**
- Open Visual Studio Installer
- Search for "Spectre" in Individual Components
- Check libraries for required architectures

**Error: "ntddk.h not found"**
- Verify WDK is installed correctly
- Add WDK paths to project in VC++ Directories

**Slow compilation**
- Use Release configuration instead of Debug
- Enable Multi-processor Compilation

## License

[Specify license as needed]
