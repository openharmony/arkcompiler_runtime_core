/*---
desc: Direct readonly field initialization.
assert: A variable with the readonly modifier may only be initialized once, its value could not be modified after initialization.
tags: [negative, compile-only]
---*/

class A {
  public readonly x;
}
