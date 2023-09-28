/*---
desc: Overload two methods, one declared and one inherited.
assert: If two methods of a class (whether both declared in the same class, or both inherited by a class, or one declared and one inherited) have the same name but signatures that are not override-equivalent, then the method name is said to be overloaded. This fact causes no difficulty and never of itself results in a compile-time error.
---*/

class A {
  public foo(): int {
    return 1;
  }
}

class B extends A{
  public foo(a: int): int {
    return a;
  }
}

function main(): void {
  let instance: B = new B();

  assert instance.foo() == 1;
  assert instance.foo(2) == 2;
}
