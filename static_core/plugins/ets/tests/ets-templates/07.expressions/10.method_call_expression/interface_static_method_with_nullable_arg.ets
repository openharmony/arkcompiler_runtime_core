/*---
desc: Call instance method of a interface with ?? argument.
assert: A compile-time error occurs if typeArguments is present and any of the type arguments are wildcards (§??).
tags: [negative]
---*/

interface A {
  static bar(x: int): int {
    return x * 2;
  }
}

class B implements A {}

function main(): void {
  let x: int? = 3;
  assert new B().bar(x) == 6;
}