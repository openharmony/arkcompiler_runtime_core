/*---
desc: Field getter declaration with value type.
---*/

class A {
  private _age: int = 0;

  get age(): int {
    return this._age;
  }
}

function main(): void {
  let instance: A = new A();

  assert instance.age == 0;
}