/*---
desc: Test with an object incompatible with the specified type
tags: [negative]
---*/

class A {}

function main(): void {
  let a = new A();
  assert a instanceof String;
}