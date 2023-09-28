/*---
desc: null safety -> Not-null operator
assert: For an expression e!, if the type of e is nullable, an ensure not-null expression checks whether the evaluation of e is equal to null and, if it is, throws a NullPointerException. 
---*/

function foo(arg: Object) : int { 
  return 1; 
}

function foo(arg: Object|null) : int { 
  return 2; 

}

function main(): void {
  let a: Object|null = new Object();
  let b: Object|null = null;

  assert foo(a) == 2;
  assert foo(a!) == 1;
  assert foo(b) == 2;
}
