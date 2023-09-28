/*---
desc: Checking a protected field from a derived class inside a composite class
---*/

class Person {
  protected name: String;
  age: int;
}

class Child extends Person {
  static factory(): Child {
    let child: Child = {name:"Bob", age: 18};

    return child;
  }
}

function main(): void {
  let obj: Child = Child.factory();

  assert obj.name == "Bob";
}