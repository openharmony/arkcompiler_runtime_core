/*---
desc: Declaring local class in function body and create instance of it.
assert: A local class can only be referred to with a simple name (not a qualified or canonical name), i.e., a local class cannot be considered if a canonical name is required.
tags: [compile-only]
---*/

function bar(): void {
  inner class B {}

  let x: B = new B();
}