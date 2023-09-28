{% for c in cases %}
/*---
desc: >-
    If two functions declared in the same declaration space
    have the same name but different signatures that are not
    override-equivalent, then the function name is said to be overloaded.
---*/

{{c.decl}}

function main(): int {
  if ({{c.cond}}) return 0;
  return 1;
}
{% endfor %}
