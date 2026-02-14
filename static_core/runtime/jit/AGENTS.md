# Runtime JIT Profiling

This directory handles runtime profiling data collection, persistence, and loading.

## Directory Structure

- `profiling_data.h` - Runtime profiling data structures:
  - `CallSiteInlineCache` - Receiver type cache for virtual call sites (up to 4 classes, marked MEGAMORPHIC when exceeded), thread-safe via atomic operations
  - `BranchData` - Branch taken/not-taken counters (atomic relaxed)
  - `ThrowData` - Exception throw frequency counters
  - `ProfilingData` - Container for the above, holds Span arrays sorted by bytecode PC
- `libprofile/` - `.ap` profile file format:
  - `pgo_header.h` - Binary format definitions (PgoHeader, MethodsHeader, MethodDataHeader, AotCallSiteInlineCache, AotBranchData, AotThrowData)
  - `pgo_file_builder.{h,cpp}` - File serialization/deserialization
  - `aot_profiling_data.h` - Serializable data structures for AOT
- `profiling_loader.{h,cpp}` - Load `.ap` profile files into runtime structures
- `profiling_saver.{h,cpp}` - Serialize runtime profiling data to `.ap` format
- `profile_saver_worker.{h,cpp}` - Background async profile saving worker (based on taskmanager::TaskQueue)

## Build

Sources are compiled into the `arkruntime` target.
