/*---
desc: Field getter with override modifier
---*/

class A {
  protected _age: int = 5;

  get age(): int {
    return this._age;
  }
}

class B extends A {
  override get age(): int {
    return this._age + 5;
  }
}

function main(): void {
  let instance: B = new B();
  assert instance.age == 10;

  let instance2: A = new B();
  assert instance2.getAge() == 10;
}