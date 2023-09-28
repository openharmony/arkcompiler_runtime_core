/*---
desc: >-
    Array access expression. The type of array reference expression must be an array type, or a compile-
    time error occurs.
tags: [compile-only, negative]
---*/

function getArrRef(): Int|null {
    return new Int(1);
}

function main(): void {
  assert getArrRef()?.[0] == null;
}
