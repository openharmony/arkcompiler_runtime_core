/*---
desc: Field setter with static modifier
---*/

class A {
  private static _age: int = 0;

  static set age(value: int) {
    A._age = value;
  }

  static getAge(): int {
    return A._age;
  }
}

function main(): void {
  A.age = 5;
  assert A.getAge() == 5;
}