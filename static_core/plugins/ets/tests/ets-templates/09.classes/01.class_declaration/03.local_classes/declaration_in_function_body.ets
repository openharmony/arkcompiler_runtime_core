/*---
desc: Declaring local class in function body.
assert: Local classes are defined between balanced braces in a group of zero or more statements (i.e., in a block that is a method body, a for loop or an if clause).
tags: [compile-only]
---*/

function bar(): void {
  let x: int;

  inner class B {
    public bar(): void {
      let y: int = x + 1;
    }
  }
}