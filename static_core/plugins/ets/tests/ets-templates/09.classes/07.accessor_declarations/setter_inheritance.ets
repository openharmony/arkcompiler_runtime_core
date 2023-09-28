/*---
desc: Field setter inheritance by child class
---*/

final class A {
  protected _age: int = 0;

  set age(value: int) {
    this._age = value;
  }

  public getAge(): Int {
    return this._age;
  }
}

class B extends A { }

function main(): void {
  let instance: B = new B();
  instance.age = 5;
  instance.getAge() == 5;
}