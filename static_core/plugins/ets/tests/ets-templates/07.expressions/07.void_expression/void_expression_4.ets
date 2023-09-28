/*---
desc: Method call of void type with empty return value
---*/

let checkVar: int;

class MyClass {
  myMethod(): void {
    checkVar = 1;
    return
  }
}

let myClass = new MyClass()

function main(): void {
  checkVar = 0;
  myClass.myMethod();
  assert checkVar == 1;
}