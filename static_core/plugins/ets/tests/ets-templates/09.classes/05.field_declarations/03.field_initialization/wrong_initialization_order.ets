/*---
desc: Check that readonly static field initialized first.
assert: Note that static fields that are readonly variables are initialized before other static fields.
tags: [negative, compile-only]
---*/

class A {
  public static readonly y: int = x;

  public static x: int = 5;
}
