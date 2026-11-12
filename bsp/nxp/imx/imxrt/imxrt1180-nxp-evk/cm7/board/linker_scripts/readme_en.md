# CM7 Linker Script Layout

[Chinese version](readme.md)

This document describes the memory layouts implemented by the MIMXRT1189 CM7 linker files in this directory, in Kconfig/Keil target order.
Addresses are from the CM7 perspective. End addresses in tables are inclusive; KB and MB mean 1024 and 1048576 bytes, respectively.
Tables describe region capacities or reservations, not the actual size of the current firmware. Use the linker map for actual usage.

A linker file tells the linker where to place code, constants, initialized data, uninitialized data, stacks, and heaps.
It also defines symbols used by startup and board code, and may describe where initialization values are stored before they are copied to RAM.
GCC uses linker scripts (`.ld`), Keil ARMCLANG uses scatter-loading files (`.scf`), and IAR uses linker configuration files (`.icf`).
A linker file does not, by itself, initialize memory controllers, configure the MPU/TRDC, or implement the complete boot process.

## 1. Files and Targets

| Kconfig type | Keil target | GCC | Keil ARMCLANG | IAR |
| --- | --- | --- | --- | --- |
| `RAM` | `rtthread_ram` | [MIMXRT1189xxxxx_cm7_ram.ld](MIMXRT1189xxxxx_cm7_ram.ld) | [MIMXRT1189xxxxx_cm7_ram.scf](MIMXRT1189xxxxx_cm7_ram.scf) | [MIMXRT1189xxxxx_cm7_ram.icf](MIMXRT1189xxxxx_cm7_ram.icf) |
| `RAM_INDUSTRY` | `rtthread_ram_industry` | [MIMXRT1189xxxxx_cm7_ram_industry.ld](MIMXRT1189xxxxx_cm7_ram_industry.ld) | [MIMXRT1189xxxxx_cm7_ram_industry.scf](MIMXRT1189xxxxx_cm7_ram_industry.scf) | [MIMXRT1189xxxxx_cm7_ram_industry.icf](MIMXRT1189xxxxx_cm7_ram_industry.icf) |
| `HYPERRAM` | `rtthread_hyperram` | [MIMXRT1189xxxxx_cm7_hyperram.ld](MIMXRT1189xxxxx_cm7_hyperram.ld) | [MIMXRT1189xxxxx_cm7_hyperram.scf](MIMXRT1189xxxxx_cm7_hyperram.scf) | [MIMXRT1189xxxxx_cm7_hyperram.icf](MIMXRT1189xxxxx_cm7_hyperram.icf) |
| `FLEXSPI_NOR` | `rtthread_flexspi_nor` | [MIMXRT1189xxxxx_cm7_flexspi_nor.ld](MIMXRT1189xxxxx_cm7_flexspi_nor.ld) | [MIMXRT1189xxxxx_cm7_flexspi_nor.scf](MIMXRT1189xxxxx_cm7_flexspi_nor.scf) | [MIMXRT1189xxxxx_cm7_flexspi_nor.icf](MIMXRT1189xxxxx_cm7_flexspi_nor.icf) |
| `FLEXSPI_NOR_HYPERRAM` | `rtthread_flexspi_nor_hyperram` | [MIMXRT1189xxxxx_cm7_flexspi_nor_hyperram.ld](MIMXRT1189xxxxx_cm7_flexspi_nor_hyperram.ld) | [MIMXRT1189xxxxx_cm7_flexspi_nor_hyperram.scf](MIMXRT1189xxxxx_cm7_flexspi_nor_hyperram.scf) | [MIMXRT1189xxxxx_cm7_flexspi_nor_hyperram.icf](MIMXRT1189xxxxx_cm7_flexspi_nor_hyperram.icf) |
| `FLEXSPI_NOR_INDUSTRY` | `rtthread_flexspi_nor_industry` | [MIMXRT1189xxxxx_cm7_flexspi_nor_industry.ld](MIMXRT1189xxxxx_cm7_flexspi_nor_industry.ld) | [MIMXRT1189xxxxx_cm7_flexspi_nor_industry.scf](MIMXRT1189xxxxx_cm7_flexspi_nor_industry.scf) | [MIMXRT1189xxxxx_cm7_flexspi_nor_industry.icf](MIMXRT1189xxxxx_cm7_flexspi_nor_industry.icf) |

The `BSP_LINKER_SCRIPT_*` choice in [board/Kconfig](../Kconfig) selects the type; its default is `FLEXSPI_NOR`.
[rtconfig.py](../../rtconfig.py) reads the generated configuration header and selects `.ld`, `.scf`, or `.icf` according to the toolchain. If no matching configuration macro is found, it falls back to `RAM`.
When building directly in an IDE, also check the linker file and linker symbols used by the active target.

## 2. Base Memory Map

| Memory | Start | End | Capacity | Description |
| --- | --- | --- | --- | --- |
| ITCM | `0x00000000` | `0x0003FFFF` | 256KB | Internal code RAM: vectors/code for RAM targets, quick-access code for XIP targets |
| DTCM | `0x20000000` | `0x2003FFFF` | 256KB | Internal data RAM: ordinary data or `DataQuickAccess` |
| Accessible OCRAM1 | `0x20484000` | `0x204FFFFF` | 496KB | The first 16KB of the physical 512KB is blocked by TRDC and excluded by the linker files |
| OCRAM2 | `0x20500000` | `0x2053FFFF` | 256KB | Shared region in standard layouts, or partitioned between cores in industry layouts |
| CM7 HyperRAM window | `0x04000000` | `0x045FFFFF` | 6MB | File comments reserve 2MB of the physical 8MB for CM33 |
| Declared SDRAM window | `0x80000000` | `0x81DFFFFF` | 30MB | File comments reserve 2MB for CM33; none of these six layouts places its main sections in SDRAM |
| Flash window declared by RAM layouts | `0x28000000` | `0x28FFFFFF` | 16MB | Declared in `RAM`, `RAM_INDUSTRY`, and `HYPERRAM`, but not used as their code execution region |
| Flash window used by XIP layouts | `0x28800000` | `0x28FFFFFF` | 8MB | Vectors, code, and initialization load data for the three `FLEXSPI_NOR*` targets |

Declaring `m_sdram_*`, `m_flash_*`, or `m_hyperram_*` does not necessarily allocate output sections there.
Quick-access TCM regions can overlap the address windows used for ordinary TCM sections. Do not count their capacities twice.

## 3. Layout Overview

| Type | Vector table | Ordinary code/read-only data | Ordinary RW/ZI data and main stack | Quick-access data | OCRAM allocation |
| --- | --- | --- | --- | --- | --- |
| `RAM` | ITCM | ITCM | DTCM | DTCM, combined with ordinary data | Standard |
| `RAM_INDUSTRY` | ITCM | ITCM | DTCM | DTCM, combined with ordinary data | Industry |
| `HYPERRAM` | ITCM | ITCM | HyperRAM | DTCM, separately placed | Standard |
| `FLEXSPI_NOR` | Flash | Flash XIP | DTCM | DTCM, combined with ordinary data | Standard |
| `FLEXSPI_NOR_HYPERRAM` | Flash | Flash XIP | HyperRAM | DTCM, separately placed | Standard |
| `FLEXSPI_NOR_INDUSTRY` | Flash | Flash XIP | DTCM | DTCM, combined with ordinary data | Industry |

The main stack is the startup/runtime-library stack, not the separate stack of each RT-Thread thread.
Likewise, the linker heap is not necessarily the complete runtime heap. Thread stacks and additional memheaps depend on runtime initialization.

### 3.1 RAM

| Region | Start | End | Capacity/purpose |
| --- | --- | --- | --- |
| Vector table | `0x00000000` | `0x000003FF` | 1KB |
| Code, read-only data, and initialization load data | `0x00000400` | `0x0003FFFF` | 255KB; initialization load data also consumes this space |
| Ordinary data, BSS, quick-access data, main stack | `0x20000000` | `0x2003FFFF` | 256KB DTCM |

`CodeQuickAccess` is included in the ordinary ITCM code region, not allocated a separate code RAM block.
The file declares a Flash base and `app_image_offset = 0xB000`, but the vector table is in ITCM, not at `0x2800B000`.
This layout alone does not provide a complete standalone Flash boot image. A debugger or multicore boot flow must load the RAM image.
See Section 4 for OCRAM1 heap/non-cacheable and OCRAM2 shared memory.

### 3.2 RAM_INDUSTRY

The ITCM/DTCM vector, code, ordinary data, and main-stack layout is the same as `RAM`.
The main changes are an expanded CM7 OCRAM1 allocation and industrial Ethernet partitioning of OCRAM2:

- Default heap capacity increases from 32KB to 64KB. The heap window starts at `0x20484000` instead of the standard multicore layout's `0x204C0000`.
- The OCRAM1 non-cacheable window is fixed at 256KB starting at `0x204C0000`.
- OCRAM2 reserves 96KB starting at `0x20520000` for CM7 and its final 32KB for shared memory.
- HyperRAM is not required; code still executes from internal ITCM.

GCC reserves the full 96KB using `.ocram2_data (NOLOAD)`, Keil uses `OCRAM2_DATA ... EMPTY`, and IAR uses an empty fixed-size `OCRAM2_DATA` block.
This reserves extra RAM; it does not automatically move `.data`/`.bss` into OCRAM2 or register an RT-Thread memheap.

### 3.3 HYPERRAM

Vectors and ordinary code use the same layout as `RAM`; ordinary data moves to external HyperRAM.

| Region | Start | End | Capacity/purpose |
| --- | --- | --- | --- |
| Vector table | `0x00000000` | `0x000003FF` | 1KB |
| Ordinary code/read-only data and initialization load data | `0x00000400` | `0x0003FFFF` | 255KB ITCM |
| `DataQuickAccess` | `0x20000000` | `0x2003FFFF` | 256KB DTCM window |
| Ordinary data, BSS, main stack | `0x04000000` | `0x045FFFFF` | 6MB HyperRAM |

`CacheLineData.init` and `CacheLineData` also reside in HyperRAM, with 32-byte cache-line alignment handled by the linker files.
The ordinary heap remains in OCRAM1; it does not move to HyperRAM with `.data`.
Because the main stack and data initialization destinations can be in external RAM, the HyperRAM controller and access permissions must be ready before those locations are used. Selecting a linker file does not initialize the peripheral.

### 3.4 FLEXSPI_NOR

| Region | Start | End | Capacity/purpose |
| --- | --- | --- | --- |
| Flash vector table | `0x28800000` | `0x288003FF` | 1KB; `app_image_offset = 0` |
| Flash ordinary code, read-only data, and initialization load data | `0x28800400` | `0x28FFFFFF` | 8MB minus 1KB |
| Quick-access code ITCM window | `0x00000000` | `0x0003FFFF` | 256KB; starts at `0x00000400` when RAM vectors are enabled |
| Ordinary data, BSS, quick-access data, main stack | `0x20000000` | `0x2003FFFF` | 256KB DTCM |

Ordinary code executes in place (XIP). Initialized RW data runs in DTCM, but its initial values are stored in the Flash image.
GCC's `.ram_function` explicitly collects `CodeQuickAccess` and the `.text*` sections of `fsl_flexspi.o`, `drv_flexspi_nor_flash.o`, and `drv_flexspi.o`. Keil explicitly selects their code in `ER_m_QuickAccessCode`.
Such code must remain executable while Flash is being reconfigured and cannot rely on the XIP window being reconfigured.
IAR explicitly places `CodeQuickAccess` in `QACODE_region` and includes the three driver objects' RO code in its copy-initialization rules. Verify actual driver execution placement in the IAR map; a copy rule alone does not prove that all driver code resides in ITCM.

### 3.5 FLEXSPI_NOR_HYPERRAM

Flash vectors, Flash code, and ITCM quick-access code use the same layout as `FLEXSPI_NOR`.
Ordinary data, BSS, `CacheLineData*`, and the main stack use the 6MB HyperRAM window from `0x04000000` to `0x045FFFFF`.
`DataQuickAccess` remains in the DTCM window from `0x20000000` to `0x2003FFFF`.
OCRAM1 heap/non-cacheable and OCRAM2 shared memory retain the standard layout.

This configuration depends on both NOR XIP and HyperRAM. Code must remain executable during Flash reconfiguration, and HyperRAM must be accessible before runtime initialization or main-stack use.
The Flash image contains ordinary code, RW initial values, and quick-access code/data initial values. Flash `.text` size alone is not the total image requirement.

### 3.6 FLEXSPI_NOR_INDUSTRY

Flash vectors, XIP code, ITCM quick-access code, DTCM ordinary data, and the main stack use the same layout as `FLEXSPI_NOR`.
OCRAM1/OCRAM2 use the industry partition shared with `RAM_INDUSTRY`, with a default 64KB heap.

GCC reserves the additional 96KB in `.ocram2` by advancing the location counter. This section has no explicit `NOLOAD`, unlike `.ocram2_data (NOLOAD)` in `RAM_INDUSTRY`; inspect the resulting ELF/binary properties.
Keil uses `RW_m_ocram2 ... EMPTY`; IAR uses an empty fixed-size block.
This target's GCC file does not directly define `__use_shmem__` at the top. Valid exported shared-memory addresses depend on the linker receiving that symbol; see Section 7.

## 4. Standard OCRAM Layout

Applies to `RAM`, `HYPERRAM`, `FLEXSPI_NOR`, and `FLEXSPI_NOR_HYPERRAM`.
The following table assumes `__multicore__` is defined and `__heap_noncacheable__` is not defined:

| Region | Start | End | Capacity/purpose |
| --- | --- | --- | --- |
| Lower OCRAM1 not assigned to this CM7 layout | `0x20484000` | `0x204BFFFF` | 240KB; coordinate ownership with CM33 in multicore use |
| CM7 cacheable heap window | `0x204C0000` | `0x204DFFFF` | 128KB window; actual default heap is 32KB |
| CM7 non-cacheable window | `0x204E0000` | `0x204FFFFF` | 128KB |
| RPMsg shared window | `0x20500000` | `0x2053FFFF` | 256KB; shared-memory symbols must be enabled |

A 128KB heap window does not mean a 128KB heap is allocated. All three toolchains default to 32KB; GCC/Keil reserve `0x204C0000` through `0x204C7FFF` by default.
IAR places the HEAP block at the end of HEAP_region, so its actual heap addresses are not the same as GCC/Keil's.

Without `__multicore__`, the standard heap window becomes `0x20484000` through `0x204BFFFF`, and the non-cacheable window becomes 256KB from `0x204C0000` through `0x204FFFFF`.
The shared window remains 256KB, but generating/enabling the shared section depends on `__use_shmem__`, which is separate from `__multicore__`.

## 5. Industrial Ethernet OCRAM Layout

Applies to `RAM_INDUSTRY` and `FLEXSPI_NOR_INDUSTRY`. Their main partition sizes do not change with `__multicore__`.
The following table assumes a cacheable heap:

| Region | Start | End | Capacity/purpose |
| --- | --- | --- | --- |
| CM7 cacheable heap window | `0x20484000` | `0x204BFFFF` | 240KB; default heap is 64KB |
| CM7 non-cacheable window | `0x204C0000` | `0x204FFFFF` | 256KB for DMA/industrial Ethernet data |
| CM33-exclusive OCRAM2 | `0x20500000` | `0x2051FFFF` | 128KB; CM7 must not use this area |
| CM7-exclusive additional OCRAM2 | `0x20520000` | `0x20537FFF` | 96KB for extra buffers or a runtime memheap |
| Multicore RPMsg shared window | `0x20538000` | `0x2053FFFF` | 32KB |

GCC/Keil's default cacheable heap spans `0x20484000` through `0x20493FFF`. The remaining 176KB is not automatically added to the heap.
With a cacheable heap and the non-cacheable partition unchanged, the heap window allows growth up to 240KB. Check linker constraints and runtime heap initialization together.
IAR still places HEAP at the end of HEAP_region; its actual block need not start at the window's base.

```text
OCRAM1: 0x20484000             0x204C0000                 0x20500000
        | cacheable 240KB     | non-cacheable 256KB      |
        | default heap 64KB   |                         |

OCRAM2: 0x20500000       0x20520000         0x20538000     0x20540000
        | CM33 128KB    | CM7 96KB         | shared 32KB  |
```

The rightmost diagram addresses are one byte past the corresponding regions.
For multicore industry use, check the [CM33 RAM_INDUSTRY linker file](../../../cm33/board/linker_scripts/MIMXRT1189xxxxx_cm33_ram_industry.ld) and matching toolchain configuration together. Do not leave CM33 using the standard shared layout while changing only CM7 to the industry partition.

### 5.1 Symbols for the Additional 96KB

| Toolchain | RAM_INDUSTRY | FLEXSPI_NOR_INDUSTRY |
| --- | --- | --- |
| GCC | `__OCRAM2_DATA_START`, `__OCRAM2_DATA_SIZE` | Same; also `__ocram2_data_start`, `__ocram2_data_end` |
| Keil | `Image$$OCRAM2_DATA$$Base`, `Image$$OCRAM2_DATA$$Limit` | `Image$$RW_m_ocram2$$Base`, `Image$$RW_m_ocram2$$Limit` |
| IAR | `__OCRAM2_DATA_START`, `__OCRAM2_DATA_SIZE` | Same |

`START/SIZE` are `0x20520000/0x18000`. End/limit symbols generally denote the first address after the region; follow the respective toolchain's semantics.
Symbol names are not identical across toolchains. Runtime code initializing an additional heap must use the appropriate symbols.

## 6. Default Heap and Main Stack

Values below apply when `__heap_size__` and `__stack_size__` are not supplied to the linker. Each cell is "heap / main stack".

| Type | GCC `.ld` | Keil `.scf` | IAR `.icf` |
| --- | --- | --- | --- |
| `RAM` | 32KB / 4KB | 32KB / 4KB | 32KB / 4KB |
| `RAM_INDUSTRY` | 64KB / 4KB | 64KB / 4KB | 64KB / 4KB |
| `HYPERRAM` | 32KB / 4KB | 32KB / 4KB | 32KB / 4KB |
| `FLEXSPI_NOR` | 32KB / 4KB | 32KB / 4KB | 32KB / 4KB |
| `FLEXSPI_NOR_HYPERRAM` | 32KB / 4KB | 32KB / 4KB | 32KB / 4KB |
| `FLEXSPI_NOR_INDUSTRY` | 64KB / 4KB | 64KB / 4KB | 64KB / 4KB |

The main-stack top is one byte past the ordinary data region: `0x20040000` for DTCM-data targets and `0x04600000` for HyperRAM-data targets.
The default 4KB stack occupies `0x2003F000` through `0x2003FFFF`, or `0x045FF000` through `0x045FFFFF`.
Ordinary RW/ZI data must leave room for the main stack; it cannot consume the entire data region.

Defining `__heap_noncacheable__` moves the heap into the OCRAM1 non-cacheable window:

- GCC deducts the heap size from the end of the non-cacheable data allocation and creates a separate heap MEMORY region. The exported MPU non-cacheable size remains the full window size.
- Keil places the heap after actual non-cacheable data with `0x100` alignment; it is not necessarily fixed at the window's end.
- IAR places HEAP at the end of NCACHE_region, sharing its capacity with NCACHE_VAR.

The combined non-cacheable data and heap usage must fit the window. Check the map and MPU setup to ensure the resulting addresses retain the intended attributes.

## 7. Sections, Load Addresses, and Linker Symbols

### 7.1 Execution and Load Addresses

The execution address (VMA) determines where the CPU accesses a section. The load address (LMA) determines where its initialization contents reside in the image.
GCC uses `AT(...)` for `.data`, `.ram_function`, `.ncache.init`, and `.qadata`, placing their initial contents consecutively after code load contents.
RAM-layout initialization contents are loaded as part of the ITCM image; XIP-layout initialization contents reside in the Flash image.
Consequently, RAM targets must fit both code and initialization copies into the 255KB code/load window.
GCC's `text_data_end` ASSERT checks that code and initialization load contents do not exceed `m_text`.
Keil describes copies with load/execution regions; IAR uses `initialize by copy`. Their maps express these relationships differently.

| Content | GCC section/input selection | Keil/IAR concept | Purpose |
| --- | --- | --- | --- |
| Vectors | `.interrupts` / `.isr_vector` | `VECTOR_ROM` / `.intvec` | Initial MSP and interrupt entries |
| Read-only code/constants | `.text`, `.rodata*` | `+RO` / `readonly` | ITCM or Flash XIP |
| Quick-access code | `CodeQuickAccess` / `.ram_function` | `ER_m_QuickAccessCode` / `QACCESS_CODE` | ITCM execution |
| Ordinary initialized data | `.data*` | `+RW` / `readwrite` | Copied to data RAM during startup |
| Ordinary uninitialized data | `.bss*`, `COMMON` | `+ZI` / `zi` | Zeroed during startup |
| Quick-access data | `DataQuickAccess` | QuickAccessData / `QACCESS_DATA` | DTCM |
| Non-cacheable data | `NonCacheable.init`, `NonCacheable` | `.bss.NonCacheable`, etc. / `NCACHE_VAR` | Buffers requiring DMA coherency |
| Shared memory | `.noinit.$rpmsg_sh_mem` | `rpmsg_sh_mem_section` | Multicore RPMsg; avoid unintended initialization |
| RT-Thread registration tables | `FSymTab`, `VSymTab`, `.rti_fn*` | Toolchain-specific sections | Shell symbols and component initialization |

GCC explicitly KEEP-s RT-Thread registration tables. Its shared `.rpmsg` output section has no explicit `NOLOAD`; check input-section attributes and startup code rather than assuming every name containing `.noinit` has identical protection.
IAR explicitly uses `do not initialize` for `.noinit` and, when enabled, `rpmsg_sh_mem_section`.
A region marked RW or named non-cacheable does not automatically configure MPU/cache/TRDC attributes; startup and board code establish those properties.

### 7.2 Symbol Semantics

| Symbol | Effect |
| --- | --- |
| `__stack_size__` | Overrides default main-stack size |
| `__heap_size__` | Overrides default heap size |
| `__multicore__` | Standard OCRAM1 allocation: 128KB NCACHE for multicore, 256KB otherwise |
| `__use_shmem__` | Enables shared sections/shared-address symbols |
| `__heap_noncacheable__` | Moves the heap into the non-cacheable OCRAM1 window |
| `__ram_vector_table__` | Reserves 1KB at the start of ITCM for XIP targets; quick-access code starts after it |

These conditions generally test whether a symbol is defined, not whether its value is 1. Defining a symbol as 0 can still enable the branch.
They are linker/preprocessor symbols; adding a same-named macro to a C configuration header alone does not necessarily make them effective.

The current GCC files for `RAM`, `RAM_INDUSTRY`, `HYPERRAM`, `FLEXSPI_NOR`, and `FLEXSPI_NOR_HYPERRAM` directly define `__use_shmem__ = 1` and `__multicore__ = 1`.
`FLEXSPI_NOR_INDUSTRY.ld` does not directly define those two symbols. Its industry partition is fixed, but without `__use_shmem__` the exported `__RPMSG_SH_MEM_START/SIZE` are zero even though MEMORY still declares a shared region.
Keil/IAR files rely on the project supplying these symbols. Determine the final layout using both the file and the project's linker parameters.

GCC uses `__RAM_VECTOR_TABLE_SIZE_BYTES`; IAR exports `__RAM_VECTOR_TABLE_SIZE`; Keil represents the reservation with the `VECTOR_RAM` execution region.
RAM-layout targets already place vectors in ITCM and reserve zero bytes for an additional RAM vector table. For XIP, a nonzero reservation only establishes space; startup code must still copy vectors and update VTOR.

## 8. Selection and Modification Checklist

- Choose `RAM` for internal-RAM debugging without industry partitioning.
- Choose `RAM_INDUSTRY` for internal-RAM execution with the industrial multicore partition.
- Choose `HYPERRAM` for ITCM execution with larger external RAM for ordinary data.
- Choose `FLEXSPI_NOR` for NOR XIP with ordinary data in internal DTCM.
- Choose `FLEXSPI_NOR_HYPERRAM` for NOR XIP with ordinary data in HyperRAM.
- Choose `FLEXSPI_NOR_INDUSTRY` for NOR XIP with industrial Ethernet partitioning and 96KB of CM7-exclusive OCRAM2.

When changing a layout, update the matching `.ld/.scf/.icf` files together and check:

1. The active target, configuration macro, actual linker file, and linker symbols agree.
2. Vector addresses match debugger and CM33-to-CM7 boot conventions. XIP address `0x28800000` is the CM7 window; do not infer the entire Flash boot-header layout from this linker file alone.
3. Flash/ITCM load regions fit code, read-only data, and all initialization copies. Data regions fit RW, ZI, alignment padding, and the main stack.
4. OCRAM1 avoids the first 16KB blocked by TRDC; OCRAM2 ownership matches CM33 without overlap.
5. Non-cacheable and shared MPU window sizes are powers of two, with base addresses aligned to their sizes. Do not treat the industry's additional 96KB region as a single power-of-two MPU region.
6. HyperRAM/FlexSPI and access permissions are initialized before affected sections or stacks are used; copying/zeroing agrees with startup code.
7. Runtime code actually registers any required additional 96KB memheap and uses symbols appropriate to the toolchain.
8. Inspect actual VMA/LMA, heap/stack boundaries, and shared-section attributes in the map. Successful linking alone does not prove DMA/cache or multicore coherency is correct.