/*---
desc: Field setter with private modifier. call out class
tags: [compile-only, negative]
---*/

class A {
  private _age: int = 0;

  private set age(value: int) {
    this._age = value;
  }

  public getAge(): int {
    return this._age;
  }
}

function main(): void {
  let instance: A = new A();
  instance.age = 5;
  assert instance.getAge() == 5;
}