/*---
desc: Field getter inheritance by child class
---*/

class A {
  protected _age: int = 5;

  get age(): int {
    return this._age;
  }
}

class B extends A {
}

function main(): void {
    let instance: B = new B();

    assert instance.age == 5;
}