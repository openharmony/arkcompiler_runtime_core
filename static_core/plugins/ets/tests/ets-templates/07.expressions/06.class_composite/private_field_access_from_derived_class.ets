/*---
desc: Checking a private field from a derived class inside a composite class
tags: [compile-only, negative]
---*/

class Person {
  private name: String;
  age: int;
}

class Child extends Person {
  factory(): Child {
    let child: Child = {name:"Bob", age: 18};

    return child;
  }
}