/*---
desc: Field getter declaration with reference type.
---*/

class A {}

class B {
  private _obj: A = new A();

  get obj(): A {
    return this._obj;
  }
}

function main(): void {
  let instance: B = new B();

  assert instance.obj instanceof A;
}