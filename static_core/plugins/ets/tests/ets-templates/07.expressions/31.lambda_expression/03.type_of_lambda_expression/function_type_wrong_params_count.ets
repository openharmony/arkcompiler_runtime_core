/*---
desc: lambda cast to erong function type
tags: [compile-only, negative]
---*/

type func = (y: int, x: int) => int;

function main(): void {
  let bar: func = (x: int): int => { return x; }
}