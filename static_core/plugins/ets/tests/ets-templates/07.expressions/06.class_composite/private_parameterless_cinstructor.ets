/*---
desc: Compilation error if constructor is not accessible
tags: [negative, compile-only]
---*/

class MyClass {
  private constructor() {}
}

function main(): void {
  let obj: MyClass = {};
}