/*---
desc: Field getter with private modifier. call out class
tags: [compile-only, negative]
---*/

class A {
  private _age: int = 5;

  private get age(): int {
    return this._age;
  }
}

function main(): void {
    let instance: A = new A();

    assert instance.age == 5;
}