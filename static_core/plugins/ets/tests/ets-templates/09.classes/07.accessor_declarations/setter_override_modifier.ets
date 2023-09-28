/*---
desc: Field setter with override modifier
---*/

class A {
  protected _age: int = 0;

  set age(value: int) {
    this._age = value;
  }

  public getAge(): Int {
    return this._age;
  }
}

class B extends A {
  override set age(value: int) {
    this._age = value + 5;
  }
}

function main(): void {
  let instance: B = new B();
  instance.age = 5;
  assert instance.getAge() == 10;

  let instance2: A = new B();
  instance2.age = 5;
  assert instance2.getAge() == 10;
}