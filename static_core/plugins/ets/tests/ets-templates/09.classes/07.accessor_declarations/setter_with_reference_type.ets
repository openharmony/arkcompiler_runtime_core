/*---
desc: Field setter declaration with reference type.
---*/

class A {}

class B {
  private _obj: A;

  set obj(value: A) {
    this._obj = value;
  }

  get obj(): A {
    return this._obj;
  }
}

function main(): void {
  let instance: B = new B();

  instance.obj = new A();
  assert instance.obj instanceof A;
}