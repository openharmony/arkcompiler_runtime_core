/*---
desc: Field setter with private modifier call in class.
---*/

class A {
  private _age: int = 0;

  private set age(value: int) {
    this._age = value;
  }

  public setAge(value: int): void {
    this.age = value;
  }

  public getAge(): int {
    return this._age;
  }
}

function main(): void {
  let instance: A = new A();
  instance.setAge(5);
  assert instance.getAge() == 5;
}