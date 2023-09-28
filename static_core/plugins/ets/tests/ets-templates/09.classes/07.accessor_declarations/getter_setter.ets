/*---
desc: Field getter and setter declaration with value type.
---*/

class A {
  private _age: int;

  get age(): int {
    return this._age;
  }

  set age(value: int) {
    this._age = value;
  }
}

function main(): void {
    let instance: A = new A();

    instance.age = 15;
    assert instance.age == 15;
}