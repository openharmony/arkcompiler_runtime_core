# Interop Serialization Knowledge

This document records the specification rules for JSON serialization across ArkTS-Sta and ArkTS-Dyn contexts.

## Core Principles

1. Serialization object types (including nested properties) must be within the supported type mapping scope.
2. When Dyn context uses its built-in `JSON.stringify` to serialize a Sta object, the result matches what `JSON.stringify` would produce in the Sta context.
3. Dyn objects in Sta context: serialization result depends on the object's type mapping representation in Sta.
4. Mixed-type object graphs (containing both Sta and Dyn properties) may cause property loss, parse exceptions, or unpredictable behavior when directly serialized. **Ensure property purity before serialization.**

## Special Serialization Rules

### BigInt / long Values
- Sta `BigInt` or `long` values exceeding Dyn `Number` safe integer range (`[-2^53+1, 2^53-1]`) serialize as **strings** in Dyn's `JSON.stringify` to preserve precision.

### char Type
- Sta `char` values mapped to Dyn context, if the character is a Unicode control character (U+0000–U+001F), will be escaped with `\` per JSON spec in Dyn's `JSON.stringify` (e.g., `"\\u0000"`).

### Cross-Context JSON Access
- Dyn context can also invoke Sta's `JSON.stringify` via STValue for Sta objects:
  ```typescript
  let JSONCls = STValue.findClass("std.core.JSON");
  let stringifyRes = JSONCls.classInvokeStaticMethod('stringify', 'C{std.core.Object}:C{std.core.String}', [student]);
  let str = stringifyRes.unwrapToString();
  ```

## Example: Serializing Sta Object in Dyn Context

```typescript
// ArkTS-Sta
export let staCla: StaClass = new StaClass();
export class StaClass {
    public big: BigInt = 12345678901234567890n;  // exceeds safe range → string
    public safe: BigInt = 1234567890n;
    public value: char = c'\u0000';               // control char → escaped
    public small: int = 42;
    public name: string = "hello";
}

// ArkTS-Dyn
import { staCla } from "./static";
let json = JSON.stringify(staCla);
// {"big":"12345678901234567890","safe":1234567890,"value":"\\u0000","small":42,"name":"hello"}
```
