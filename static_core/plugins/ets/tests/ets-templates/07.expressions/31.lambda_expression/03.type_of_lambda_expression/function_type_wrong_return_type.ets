/*---
desc: lambda cast to erong function type
tags: [compile-only, negative]
---*/

type func = (x: int) => int;

function main(): void {
  let bar: func = (x: int): boolean => { return true }
}