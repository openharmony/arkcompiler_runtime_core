/*---
desc: Local declaration in statement (const)
tags: [negative, compile-only]
---*/

function main(): void {
  {
    class A {}
    let a: A = new A;
    assert a != null;
  }
  assert a != null;
}
