/*---
desc: Field setter with final modifier cant override..
tags: [compile-only, negative]
---*/

class A {
  private _age: int = 0;

  final set age(value: int){
    this._age = value;
  }
}

class B extends A {
  override set age(value: int) {
    this._age = value + 5;
  }
}
