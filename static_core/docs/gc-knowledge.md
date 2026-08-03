# GC Knowledge

This document covers only the error-prone boundaries in garbage collection and memory management. For interpreter-GC interaction (safepoints, barriers) see `interpreter-knowledge.md` in the same directory. For detailed object header and allocator design see `memory-management.md`.

## GC Types

| Type | Enum value | Generational | Moving | Concurrent | Purpose |
|------|-----------|-------------|--------|------------|---------|
| Epsilon | `EPSILON_GC` | No | No | No | Debug/perf baseline measurement |
| Epsilon-G1 | `EPSILON_G1_GC` | Yes | No | No | G1 allocator without collection |
| STW | `STW_GC` | No | No | No | Simple mark-sweep |
| G1 | `G1_GC` | Yes | Yes | Yes | Concurrent mark GC, collect young + top garbage tenured regions on MixedGC |
| CMC | `CMC_GC` | Yes | Yes | Yes | Concurrent mark-copy (common runtime) |

`--gc-type` selects the GC, but note compatibility with interpreter type (see below).

### GC and Interpreter Type Compatibility

| GC type | cpp | irtoc | llvm |
|---------|-----|-------|------|
| G1 | Supported | Supported (dynamic languages) | Supported |
| CMC | Supported | Downgraded to cpp when not explicitly set | Not supported |
| Epsilon | Supported | — | — |
| STW | Supported | — | — |

Under `ARK_USE_COMMON_RUNTIME`, the CMC GC + llvm/irtoc interpreter combination **automatically downgrades when not explicitly set**. Always specify both explicitly when debugging.

## Heap Space Model

```
MemMapSpace
+-- Code Space (executable, JIT/AOT code)
+-- Compiler Internal Space (arena linked list)
+-- Internal Memory Space (runtime internals, including GC itself)
+-- Object Space
    +-- BumpPointerSpace
    +-- Regular Object Space
    +-- Humongous Object Space
    +-- TLAB Space (optional)
    +-- RegionSpace (used by G1 etc., optional)
    +-- Non-moving Space

MallocMemSpace
+ native malloc/free
```

### Logical GC Spaces

```
+--------------+---------------------+
|  Eden/Young  |     Tenured/Old     |
|              |                     |
+--------------+---------------------+
```

## Region Model (G1 GC)

G1 is the main production GC. The heap consists of fixed-size regions:

```
+--------------+--------------+-----+--------------+
| Region #1    | Region #2    | ... | Region #N    |
| young        | tenured      |     | tenured      |
+--------------+--------------+-----+--------------+
```

### Region Types

| Type | Description |
|------|-------------|
| young | Young generation region |
| tenured | Old generation region |
| humongous | Large object region |
| empty | Free region |

### Remembered Sets

Each region maintains a Remembered Set to track cross-region references:

| Reference direction | Purpose |
|---------------------|---------|
| old -> young | Find old-to-young roots during Young GC |
| old -> old | Cross-region references during Mixed GC |

Remembered Sets can be refined by dedicated threads (`UpdateRemsetWorker`, `update_remset_thread.cpp`).

## GC Collection Flows

### Young GC (STW, young regions only)

```
1. Root scan (young gen) + Remembered Set for old gen roots
2. Mark young gen + Reference processor + move live objects to tenured
3. Sweep + finalizers
```

### Mixed GC

Young GC + some tenured regions added to the collection set after concurrent marking.

### Concurrent Marking (triggered when tenured occupancy reaches threshold)

```
1. Root scan (STW #1)
2. Concurrent marking + Reference processor
3. Remark - finish marking and update liveness statistics (STW #2)
4. Cleanup - reclaim empty regions, decide whether Mixed GC is needed
```

### Tenured GC

```
1. Initial mark - root scan (STW #1)
2. Concurrent marking + Reference processor
3. Remark (STW #2)
4. Concurrent sweep + finalizers
5. Reset
```

## GC Barriers

GC barriers ensure heap consistency and optimize GC flows. Barriers fire only on **reference-type field writes**.

### Write Barriers

#### Pre Barrier

Prevents live object loss during concurrent marking:

```
if (UNLIKELY(concurrent_marking)) {
    auto pre_val = obj.field;          // read old value
    if (pre_val != nullptr) {
        store_in_buff_to_mark(pre_val); // mark the old reference
    }
}
obj.field = new_val;                   // actual write
```

#### Post Barrier

Tracks old-to-young generation references (Card Table / Remembered Set):

```
obj.field = new_val;                   // actual write
if (obj is in old gen && new_val is in young gen) {
    update_card(AddressOf(obj.field));  // mark Card Table
}
```

### Read Barrier

Used during concurrent compaction to ensure reads resolve to to-space objects:

```
obj_ref = obj.field;
if (obj_ref is forwarded) {
    obj_ref = forwarding_address;      // read to-space copy
}
```

### Do Not Mix Barrier Operations

| Barrier | When it fires | Problem it solves |
|---------|--------------|-------------------|
| Pre write barrier | Before reference write | Concurrent marking missing live objects |
| Post write barrier | After reference write | Cross-generational / cross-region reference tracking |
| Read barrier | On reference read | Concurrent compaction to-space consistency |

Do not use different barriers for read and write of the same field. For example, adding only a Pre write barrier but not handling concurrent compaction on the read side (or vice versa) will cause heap inconsistency.

## Safepoints

When GC needs STW, it pauses all mutator threads via safepoints.

### Mechanism

```
RVState register -> ExecState -> Safepoint Page
                                    |
                          GC protects this page when it needs to pause
                                    |
                          All executing threads trigger SIGSEGV
                                    |
                          Signal handler -> prepare stack object info -> wait for STW to end
                                    |
                          Return to safepoint -> continue execution
```

- Safepoints are inserted at method entry and at the head of every loop
- On IoT devices without an MMU, explicit condition checks replace memory page protection
- Interpreter frames already contain the stack object information the GC needs
- JIT/compiler-generated code requires additional stack map information for GC use

### Safepoint Flow

```
Compiled/JIT code
    |
    Safepoint #X -> load [RVState, SPaddrOffset]
    |
    SIGSEGV (page is protected)
    |
    Signal handler -> modify return PC
    |
    Stack map code (prepare object info for safepoint #X, wait for STW to end)
    |
    Jump back to safepoint #X -> normal execution resumes
```

## Mutator Model

```
+--------------+
|    Mutator   | <- Base class: entity that can mutate the managed heap
|  (abstract)  |
+------+-------+
       |
       +-- GC Tasks
       +-- ManagedThread <- can execute bytecode, has execution state
       |     +-- MTManagedThread <- one per OS thread
       +-- JIT Tasks
```

- A Mutator is not an OS thread and is not mapped 1:1 with OS threads
- GC achieves STW by notifying all Mutators to suspend/resume
- `MutatorManager` manages the lifecycle of all Mutators

## Allocators

### Size Classes

| Class | Size range | Allocation strategy |
|-------|-----------|---------------------|
| Small | 1B - 4KB | RunSlots size-segregated allocation |
| Large | 4KB - 4MB | Direct allocation (no run) |
| Humongous | 4MB+ | Proxied to OS or specialized allocator |

### Main Allocators

| Allocator | Purpose |
|-----------|---------|
| `RegionAllocator` | Region-based allocation for G1 GC, a region can contain TLAB |
| `RunSlotsAllocator` | Fast small object allocation |
| `BumpAllocator` | Bump-pointer allocation |
| `FreelistAllocator` | Free-list allocation |
| `HumongousObjAllocator` | Large object allocation |
| `CodeAllocator` | Executable memory (JIT/AOT code) |
| `ArenaAllocator` | JIT internal temporary allocation, bulk deallocation |
| `FrameAllocator` | Interpreter frame allocation |

## Pre-Change Checklist

- Are correct Pre/Post barriers inserted on reference field writes?
- Does the barrier type match the current GC configuration? (Epsilon needs none, G1 needs Pre+Post, concurrent compaction needs read barriers)
- Are safepoints present at method entry and loop heads? Does new compiler code have corresponding stack maps?
- Are object header Mark Word state bits correctly maintained? Is the forwarding address set after GC moves an object?
- Is the Remembered Set updated on cross-region reference writes?
- Does the GC correctly notify all Mutators on suspend/resume?
- Does the allocation path use TLAB? Direct global allocation increases lock contention.
- Are the GC type and interpreter type compatible?

## Code and Tests

GC core code can be traced starting from `runtime/mem/gc/`:

| Component | Anchor |
|-----------|--------|
| GC base class | `runtime/mem/gc/gc.h` |
| G1 GC | `runtime/mem/gc/g1/g1-gc.h`, `g1-gc.cpp` |
| G1 allocator | `runtime/mem/gc/g1/g1-allocator.h` |
| G1 marker | `runtime/mem/gc/g1/g1-marker.h` |
| Remembered Set update | `runtime/mem/gc/g1/update_remset_worker.h`, `update_remset_thread.cpp` |
| Hot card handling | `runtime/mem/gc/g1/hot_cards.h` |
| Pause tracking | `runtime/mem/gc/g1/g1_pause_tracker.h` |
| G1 analytics | `runtime/mem/gc/g1/g1_analytics.h` |
| Epsilon GC | `runtime/mem/gc/epsilon/epsilon.h` |
| STW GC | `runtime/mem/gc/stw-gc/stw-gc.h` |
| Generational GC base | `runtime/mem/gc/generational-gc-base.h` |
| GC barrier set | `runtime/mem/gc/gc_barrier_set.h` |
| Card Table | `runtime/mem/gc/card_table.h` |
| GC roots | `runtime/mem/gc/gc_root.h` |
| GC statistics | `runtime/mem/gc/gc_stats.h` |
| GC trigger | `runtime/mem/gc/gc_trigger.h` |
| Reference processor | `runtime/mem/gc/reference-processor/` |
| GC workers | `runtime/mem/gc/workers/` |
| Object header | `runtime/include/object_header.h`, `runtime/mark_word.h` |
| Mark Word | `runtime/mark_word.h` |
| Heap manager | `runtime/mem/heap_manager.cpp` |
| Region Space | `runtime/mem/region_space.h` |
| Heap verifier | `runtime/mem/heap_verifier.cpp` |
| Mutator | `runtime/mutator.cpp`, `runtime/mutator_manager.cpp` |
| Safepoint | `runtime/include/safepoint_timer.h` |
| GC task | `runtime/gc_task.cpp` |
| irtoc GC helpers | `irtoc/scripts/gc.irt` |

Debugging GC-sensitive runtime failures:
```bash
--log-level=debug --log-components=gc:mm-obj-events --log-detailed-gc-info-enabled=true
```

Heap verification runs automatically before and after GC (`HeapVerifier`) and can detect heap corruption. GC tests are distributed across `runtime/tests/` and `tests/tests-u-runner/`.
