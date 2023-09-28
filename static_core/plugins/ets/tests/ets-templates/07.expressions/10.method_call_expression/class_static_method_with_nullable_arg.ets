/*---
desc: Call instance method of a class with ?? argument.
assert: A compile-time error occurs if typeArguments is present and any of the type arguments are wildcards (§??).
tags: [negative]
---*/

class A {
  public static foo(x: int): int{ 
    return x * 2;
  }
}

function main(): void {
  let x: int? = 3;

  assert A.foo(x) == 6;
}