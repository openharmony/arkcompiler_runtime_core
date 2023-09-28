/*---
desc: Field getter with static modifier
---*/

class A {
  private static _age: int = 5;

  static get age(): int {
    return A._age;
  }
}

function main(): void {
  assert A.age == 5;
}