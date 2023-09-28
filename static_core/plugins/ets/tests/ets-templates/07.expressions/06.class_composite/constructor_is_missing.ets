/*---
desc: Compilation error if parameterless constructor is missing
tags: [negative, compile-only]
---*/

class MyClass {
  constructor(x: int) {}
}

function main(): void {
  let obj: MyClass = {};
}