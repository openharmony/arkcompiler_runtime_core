/*---
desc: Overload two methods in same class.
assert: When a method is invoked, the number of actual arguments (and any explicit type arguments) and the compile-time types of the arguments are used, at compile time, to determine the signature of the method that will be invoked.
tags: [compile-only]
---*/

class A {
  public foo(): int {
    return 1;
  }

  public foo(a: int): int {
    return a;
  }

  public foo(a: int, b: boolean): int {
    return a;
  }

  public foo(a: int, b: boolean, c: String): int {
    return a;
  }
}
