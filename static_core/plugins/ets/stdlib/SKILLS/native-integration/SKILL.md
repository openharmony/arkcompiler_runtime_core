---
name: native-integration
description: Essential skill covering C++ programming, FFI (Foreign Function Interface), ANI (Ark Native Interface), ICU4C integration for internationalization, and native method implementation for ETS stdlib. Use this skill when: (1) Implementing native methods, (2) Integrating with C++ code, (3) Using ICU for i18n, (4) Handling native/ETS boundary, (5) Debugging native crashes, or (6) Optimizing native code.
---

# Native Integration

## Quick Start

**Native method declaration:**
```typescript
// ETS side
export class Math {
    public native static sqrt(x: double): double;
}
```

**C++ implementation:**
```cpp
// C++ side
double Math_sqrt(double x) {
    return std::sqrt(x);
}
```

**Type conversions:**
```cpp
// ETS string to C++
std::string str = etsToCppString(etsStr);

// C++ to ETS string
EtsString* etsStr = cppToEtsString(str);
```

## Key Concepts

**ANI (Ark Native Interface):**
- StringHelper for string conversions
- ArrayHelper for array conversions
- ExceptionHelper for error handling

**ICU Integration:**
- Locale handling
- Number/date formatting
- Collation

## Script Utilities

- **check_native_integration.sh** - Validate native methods
