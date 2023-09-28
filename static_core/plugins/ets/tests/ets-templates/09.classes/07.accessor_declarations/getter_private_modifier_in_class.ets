/*---
desc: Field getter with private modifier call in class.
---*/

class A {
  private _age: int = 5;

  private get age(): int {
    return this._age;
  }

  public getAge(): int {
    return this.age;
  }
}

function main(): void {
    let instance: A = new A();

    assert instance.getAge() == 5;
}