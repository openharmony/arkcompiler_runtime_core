/*---
desc: Local declaration in statement (var)
tags: [negative, compile-only]
---*/

function main(): void {
  {
    let a: int = 0;
    assert a == 0;
  }
  assert a == 0;
}
