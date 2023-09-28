/*---
desc: Field setter declaration with value type.
---*/

class A {
  private _age: int = 0;

  set age(value: int) {
    this._age = value;
  }

  get age(): int {
    return this._age;
  }
}

function main(): void {
  let instance: A = new A();

  instance.age = 10;
  assert instance.age == 10;
}