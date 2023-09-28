/*---
desc: Declaring two local class in function body.
assert: Local can be redeclared in the context of another, more deeply nested, class such as AnotherLocal.
tags: [compile-only]
---*/

function bar(): void {
  class B {
    public bar(): void {
      inner class B {}
    }
  }
}