/*---
desc: lambda declaration with two same parameters names.
tags: [compile-only, negative]
---*/

function main(): void {
  (x: float, x: boolean): float => { return Math.sin(x) }
}