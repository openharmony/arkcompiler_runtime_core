/*---
desc: Setter throw error.
---*/

class TestException extends Exception {}

class A {
  private _age: int = 0;

  set age(value: int) throws {
    if (value < 0) { 
      throw new TestException();
    }

    this._age = value;
  }
}

function main(): void {
  let instance: A = new A();
  let catchCode = 0;

  try {
    instance.age = -5;
  } catch (e: TestException){
    catchCode = 1;
  } catch {
    assert false;
  }

  assert catchCode == 1;
}