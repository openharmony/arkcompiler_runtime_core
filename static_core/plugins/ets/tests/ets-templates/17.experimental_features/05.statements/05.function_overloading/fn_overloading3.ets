{% for c in cases %}
/*---
desc: >-
    If two functions declared in the same declaration space
    have the same name but different signatures that are not
    override-equivalent, then the function name is said to be overloaded.
params: {{c.type}}
---*/

function foo<T>(a: T): int {
  return 1;
}
function foo<T, V>(a: T, b: V): int {
  return 2;
}
function foo<T, U, V>(a: T, b: U, c: V): int {
  return 3;
}

function main(): int {
  let v: {{c.type}} = {{c.val}};
  if ({{c.call}} == {{c.res}}) return 0;
  return 1;
}
{% endfor %}
