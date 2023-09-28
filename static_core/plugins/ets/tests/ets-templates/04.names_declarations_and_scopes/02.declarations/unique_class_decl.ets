/*---
desc: A name must be unique in its class space
tags: [negative, compile-only]
---*/

class A {
  x: int;
  x: int;
}

function main(): void {}
