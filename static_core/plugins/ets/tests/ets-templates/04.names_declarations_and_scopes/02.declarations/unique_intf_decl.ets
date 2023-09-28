/*---
desc: A name must be unique in its interface space
tags: [negative, compile-only]
---*/

interface A {
  x: int = 0;
  x: int = 0;
}

function main(): void {}
