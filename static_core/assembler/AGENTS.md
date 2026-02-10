# Assembler Component

## Core Metadata

| Attribute | Value |
|-----------|--------|
| **Name** | Assembler (Panda Assembly) |
| **Purpose** | Parses Panda assembly (`.pa`) source into an in-memory **Program** (records, functions, instructions, etc.) and emits Panda bytecode (`.abc`). This directory implements the static_core assembly pipeline: lex → parse → emit. Used for hand-written or generated assembly and for tools that produce `.pa`. |
| **Primary Language** | C++ (templates/codegen: Ruby, ERB; metadata: YAML) |

## Directory Structure

```
assembler/
├── pandasm.*              # Top-level API: PrepareArgs, Tokenize, ParseProgram, EmitProgramInBinary, BuildFiles
├── lexer.*                # Lexer for .pa source
├── assembly-parser.*      # Parser: tokens → Program (records, functions, instructions)
├── assembly-program.*     # Program representation
├── assembly-emitter.*     # Program → binary .abc emission
├── assembly-*.h           # Assembly IR: type, function, field, record, ins, label, literals, methodhandle, file-location, debug (assembly-debug.h)
├── context.*, define.h, error.h, meta.*, annotation.*   # Parse context, definitions, errors, metadata, annotations
├── ide_helpers.h          # IDE/editor helpers
├── mangling.h             # Name mangling
├── metadata.yaml          # Attribute/flag definitions (single source of truth for metadata)
├── asm_isapi.rb, asm_metadata.rb   # ISA/metadata codegen scripts used with templates
├── templates/             # Codegen: isa, ins_create_api, ins_emit, ins_to_string, meta_gen, opcode_parsing, operand_types_print (.erb)
├── extensions/            # Extension hooks: MetadataExtension (create record/field/function/param metadata by language), CMake and register_extensions
├── utils/                 # number-utils, etc.
├── tests/                 # lexer, parser, emitter, type, mangling tests
└── samples/               # Sample .pa files (Bubblesort, Factorial, Fibonacci)
```

## Key Responsibilities

- **Lexer**: Tokenizes `.pa` source into token streams (per function or global scope as used by the parser).
- **Parser**: Builds **Program** from tokens (records, functions, instructions, literals, metadata).
- **Emitter** (AsmEmitter): Serializes Program to Panda binary (`.abc`); optional bytecode optimization and size statistics.
- **Metadata and ISA**: `metadata.yaml` defines attributes and flags; `.erb` templates generate ISA and instruction APIs from a single source.

## Dependencies
- **Depends on**: libarkbase (expected, logger, args, etc.); libarkfile (file format, PGO, etc.); ISA/template inputs; does not depend on ets2panda frontend.

## Extension and Modification

- **New instructions or operands**: Update ISA/template inputs and regenerate via the `templates/` ERB pipeline; parser and emitter are updated automatically — typically no manual modifications needed.
- **New attributes or metadata**: Extend **metadata.yaml** and the code that reads it (meta.*, annotation.*).
- **New assembly syntax constructs**: Extend **Parser** (assembly-parser) and **Program** representation (assembly-*.h), and update emitter binary output logic.
