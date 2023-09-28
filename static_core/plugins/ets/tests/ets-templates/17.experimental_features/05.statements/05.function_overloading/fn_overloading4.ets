{% for c in cases %}
/*---
desc: >-
    If two functions declared in the same declaration space
    have the same name but different signatures that are not
    override-equivalent, then the function name is said to be overloaded.
params: {{c.type}}
---*/

interface I {}

class A {}
class B extends A implements I {}
class C extends B {}

function foo(a: A): int {
  return 1;
}
function foo(a: B): int {
  return 2;
}
function foo(a: I): int {
  return 3;
}

function main(): int {
  let v: {{c.type}} = {{c.val}};
  if (foo(v) == {{c.res}}) return 0;
  return 1;
}
{% endfor %}
