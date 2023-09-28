/*---
desc: Overload two methods with different throws clauses.
assert: There is no required relationship between the return types or between the throws or rethrows clauses of two methods with the same name, unless their signatures are override-equivalent.
---*/

class A {
  public foo(): int throws {
    return 1;
  }

  public foo(a: int): boolean {
    return true;
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.foo() == 1;
  assert instance.foo(2) == true;
}
