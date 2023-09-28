/*---
desc: Field getter with final modifier cant override.
tags: [compile-only, negative]
---*/

final class A {
  private _age: int = 5;

  final get age(): int {
    return this._age;
  }
}

class B extends A {
  override get age(): int {
    return this._age + 5;
  }
}