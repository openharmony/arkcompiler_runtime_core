/*---
desc: Local declaration in statement (const)
tags: [negative, compile-only]
---*/

function main(): void {
  {
    const a: int = 0;
    assert a == 0;
  }
  assert a == 0;
}
