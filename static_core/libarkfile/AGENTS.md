# AGENTS.md

This file provides guidance to AI when working with code in this repository.

## Project Metadata

- **name**: Libarkfile
- **purpose**: File format library for reading and writing Ark Bytecode (.abc) files, providing accessors for classes, methods, fields, annotations, and debug information
- **primary language**: C++

## About Libarkfile

**Libarkfile** is the core library for handling Ark Bytecode (.abc) files. It provides:
- Reading and writing .abc file format
- Access to class metadata, methods, fields, annotations
- Debug information extraction and manipulation
- Bytecode instruction encoding/decoding
- PGO (Profile-Guided Optimization) data support

## Directory Structure

Main File Directories

```
libarkfile/
├── file.cpp/h                    # Main file representation
├── file_reader.cpp/h             # File reading interface
├── file_writer.cpp/h             # File writing interface
├── file_items.cpp/h              # File items container
├── file_item_container.cpp/h     # Item container management
├── file_format_version.cpp        # File format version handling
├── file-inl.h                   # Inline file functions
│
├── class_data_accessor.cpp/h      # Class metadata access
├── class_data_accessor-inl.h     # Inline class accessors
├── method_data_accessor.cpp/h     # Method metadata access
├── method_data_accessor-inl.h    # Inline method accessors
├── field_data_accessor.cpp/h      # Field metadata access
├── field_data_accessor-inl.h     # Inline field accessors
├── literal_data_accessor.cpp/h    # Literal/constant pool access
├── literal_data_accessor-inl.h   # Inline literal accessors
├── annotation_data_accessor.cpp/h # Annotation access
├── debug_data_accessor.cpp/h      # Debug information access
├── debug_data_accessor-inl.h      # Inline debug accessors
├── proto_data_accessor.cpp/h      # Prototype data access
├── proto_data_accessor-inl.h      # Inline prototype accessors
├── method_handle_data_accessor.cpp/h # Method handle access
├── param_annotations_data_accessor.h # Parameter annotations
│
├── bytecode_instruction.cpp/h      # Bytecode instruction representation
├── bytecode_instruction-inl.h     # Inline instruction functions
├── bytecode_emitter.cpp/h         # Bytecode emission for writing .abc
│
├── debug_info_extractor.cpp/h      # Extract debug information
├── debug_info_updater-inl.h       # Update debug information
├── debug_helpers.cpp/h            # Debug helper functions
│
├── helpers.h                      # Utility helper functions
├── type_helper.h                  # Type handling utilities
├── value.h                       # Value representation
├── modifiers.h                   # Method/field modifiers
├── shorty_iterator.h            # Signature type iterator
│
├── pgo.cpp/h                     # Profile-Guided Optimization data
├── panda_cache.h                 # Panda file caching
│
├── external/                      # External API bindings
│
├── templates/                     # Code generation templates
│
├── tests/                         # Unit tests
├── types.yaml                     # Type definitions for codegen
├── types.rb                      # Ruby type handling
├── pandafile_isapi.rb            # API interface definitions
├── CMakeLists.txt               # CMake build configuration
└── BUILD.gn                     # GN build configuration
```

## Key Components

### File Access
- **File**: Main class representing an .abc file in memory
- **FileReader/Writer**: Read/write .abc files from/to disk
- **FileItemContainer**: Container for file items (classes, methods, etc.)

### Data Accessors
Accessors provide type-safe views into different sections of the .abc file:
- **ClassDataAccessor**: Access class definitions (superclass, interfaces, fields, methods)
- **MethodDataAccessor**: Access method metadata (code, annotations, parameters)
- **FieldDataAccessor**: Access field metadata (type, modifiers, annotations)
- **LiteralDataAccessor**: Access constant pool (strings, numbers, types)
- **AnnotationDataAccessor**: Read annotation data
- **DebugDataAccessor**: Access debug information (line numbers, local variables)

### Bytecode Handling
- **BytecodeInstruction**: Represents a single bytecode instruction
- **BytecodeEmitter**: Emits bytecode instructions when writing .abc files
- **Instruction formats**: Defined in `isa/isa.yaml` and templates

### Debug Information
- **LineNumberProgram**: Debug line number program
- **DebugInfoExtractor**: Extract debug info from .abc files
- **DebugInfoUpdater**: Update debug information

## Build Commands

See @../AGENTS.md

## File Format (.abc)

Ark Bytecode files contain:
- **Header**: Magic number, version
- **Classes**: Class definitions with metadata
- **Methods**: Method bytecode and metadata
- **Fields**: Field definitions
- **Literal Pool**: Constants (strings, numbers, types)
- **Debug Info**: Line numbers, variable info
- **Annotations**: Class/method/field annotations
- **PGO Data**: Profile-guided optimization data

## Code Style

See @../AGENTS.md

## Testing

See @../AGENTS.md

