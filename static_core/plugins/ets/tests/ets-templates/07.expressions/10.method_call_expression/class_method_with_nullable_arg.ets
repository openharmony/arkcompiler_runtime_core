/*---
desc: Call instance method of a class.
assert: A compile-time error occurs if typeArguments is present and any of the type arguments are wildcards (§??).
tags: [negative]
---*/

class A {
  public foo(int x): int{ 
    return x * 2;
  }
}

function main(): void {
  let x: int? = 3;

  let instance: A = new A();

  assert a.foo(x) == 6;
}