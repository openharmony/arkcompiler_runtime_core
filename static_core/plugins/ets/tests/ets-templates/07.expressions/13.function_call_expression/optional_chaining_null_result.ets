/*---
desc: Optional chaining declaration with null result.
assert: if ’?.’ is provided and expression is evaluated as null then arguments are not evaluated and the call is not performed and result of the functionCallExpression is null too.
---*/

class A {
  public evaluate(value: int): Int {    
    return new Int(value);
  }
}

function getX(): Int {
  let a: A|null;
  let res = a?.evaluate(5);
  
  return res;
}

function main(): void {
  assert getX() == null;
}
