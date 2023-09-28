/*---
desc: Getter throw error.
---*/

class TestException extends Exception {}

class A {
  private _age: int = 0;

  get age(): int throws {
    if (this._age == 0) { 
      throw new TestException();
    }

    return this._age;
  }
}

function main(): void {
  let instance: A = new A();
  let catchCode = 0;

  try {
    instance.age();
  } catch (e: TestException){
    catchCode = 1;
  } catch {
    assert false;
  }

  assert catchCode == 1;
}