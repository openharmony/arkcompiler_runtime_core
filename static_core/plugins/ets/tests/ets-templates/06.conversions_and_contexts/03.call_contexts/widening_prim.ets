{% for c in cases %}
/*---
desc: Invocation contexts allow a widening primitive conversion.
params: Widening primitive conversion to '{{c.ctype}}'
---*/

class A {
  public fld1: {{c.ctype}};
  public fld2: {{c.ctype}};

  public constructor(p: {{c.ctype}}) {
    this.fld1 = p;
  }

  public foo(p: {{c.ctype}}): void {
    this.fld2 = p;
  }
}

function main(): int {
  {% for t in c['values'] %}
  let v{{loop.index}}: {{t.type}} = {{t.val|safe}};
  let obj{{loop.index}}: A = new A(v{{loop.index}});
  obj{{loop.index}}.foo(v{{loop.index}});
  if (obj{{loop.index}}.fld1 != {{t.val2|safe}} || obj{{loop.index}}.fld2 != {{t.val2|safe}}) {
    return 1;
  }
  {% endfor %}
  return 0;
}
{% endfor %}