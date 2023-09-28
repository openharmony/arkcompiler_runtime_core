{% for c in cases %}
/*---
desc: >-
    If two functions declared in the same declaration space
    have the same name but different signatures that are not
    override-equivalent, then the function name is said to be overloaded.
params: {{c.type}}
---*/

function foo({%- for i in range(c.len)%}d{{i}}: {{c.dtype}}, {%endfor%}p: {{c.type}}): int {
  return 0;
}
function foo({%- for i in range(c.len)%}d{{i}}: {{c.dtype}}, {%endfor%}p: {{c.dtype}}): int {
  return 1;
}

function main(): int {
  return foo({%- for i in range(c.len)%}{{c.dval}}, {%endfor%}{{c.val}});
}
{% endfor %}
