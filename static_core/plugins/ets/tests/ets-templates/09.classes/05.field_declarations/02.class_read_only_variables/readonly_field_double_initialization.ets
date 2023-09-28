/*---
desc: Direct and constructor readonly field initialization.
assert: A variable with the readonly modifier may only be initialized once, its value could not be modified after initialization.
tags: [negative, compile-only]
---*/

class A {
  public readonly x: int = 1;

  public constructor() {
    x = 1;
  }
}
