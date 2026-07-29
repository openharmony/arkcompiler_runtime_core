# Interop Exception and Error Mapping Knowledge

This document records the specification rules for exception/error propagation between ArkTS-Sta and ArkTS-Dyn contexts.

## Core Principles

1. Sta exceptions entering Dyn context must appear as **catchable Error objects**.
2. Cross-context Sta exception objects may only be initialized via `new` — **direct function-parameter passing is prohibited**.
3. Error objects must preserve Sta type name, message, cause, and usable stack info.
4. All Sta exceptions must not be blurred into plain strings or anonymous errors.
5. Dyn exceptions entering Sta context must appear as **catchable exception objects**.
6. Dyn exceptions entering Sta must allow access to original Error info.
7. Cross-context Dyn exception objects may only be initialized via `new` — **direct function-parameter passing is prohibited**.

## Sta → Dyn Exception Mapping

| Category | Sta Type | Dyn Type |
|---|---|---|
| Built-in Error base | Error | Error |
| Built-in Error subclasses | RangeError, ReferenceError, SyntaxError, URIError, TypeError | RangeError, ReferenceError, SyntaxError, URIError, TypeError |
| Other Error | Custom Error and other Error subclasses | Error |

## Dyn → Sta Exception Mapping

| Category | Dyn Type | Sta Type | Notes |
|---|---|---|---|
| Built-in Error base | Error | Error | — |
| Built-in Error subclasses | RangeError, ReferenceError, SyntaxError, URIError, TypeError | RangeError, ReferenceError, SyntaxError, URIError, TypeError | — |
| Other Error | Custom Error and other Error subclasses | Error | — |
| Non-Error | Dyn throws non-Error value | ESError | ESError properties accessible via internal ESValue |

## Key Interaction Patterns

- **Sta catch can capture Dyn exceptions**: Type matching works for built-in Error hierarchy.
- **Dyn catch can capture Sta exceptions**: Sta exceptions map to Dyn Error types.
- **Custom Sta Error types** lose their specific type when crossing to Dyn — they become generic `Error`.
- **Non-Error throws** from Dyn (e.g., throwing a number or string) map to `ESError` on the Sta side, with properties accessible through ESValue.

## Implementation Rules

- Both Dyn `catch` and Sta `catch` must be able to capture cross-context exceptions.
- Exception identity (type name) preservation for built-in Error types.
- Stack trace must be preserved and accessible.
- No silent error suppression — all cross-context exceptions must be catchable or explicitly propagated.
