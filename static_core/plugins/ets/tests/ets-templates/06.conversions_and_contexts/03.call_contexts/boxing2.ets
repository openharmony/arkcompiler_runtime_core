{% for c in cases %}
/*---
desc: Invocation contexts allow a boxing conversion followed by a widening reference conversion.
params: Boxing conversion from '{{c.prim_type}}' to '{{c.type}}'
---*/

class A {
  {%- for v in c['values'] %}
  public fld_{{loop.index}}: {{c.type}};
  {%- endfor %}

  public constructor({% for v in c['values'] %}p{{loop.index}}: {{c.type}}{{ ", " if not loop.last }}{% endfor %}) {
    {%- for v in c['values'] %}
    this.fld_{{loop.index}} = p{{loop.index}};
    {%- endfor %}
  }

  public foo({% for v in c['values'] %}p{{loop.index}}: {{c.type}}{{ ", " if not loop.last }}{% endfor %}): int {
    if (
      {%- for v in c['values'] %}
      this.fld_{{loop.index}} === null {{ " ||" if not loop.last }}
      {%- endfor %}
    ) {
      return 1;
    } else {
      return 0;
    }
  }
}

function main(): int {
  {%- for v in c['values'] %}
  let v{{loop.index}}: {{c.prim_type}} = {{v|safe}};
  {%- endfor %}

  let c: A = new A({% for v in c['values'] %}v{{loop.index}}{{ ", " if not loop.last }}{% endfor %});
  return c.foo({% for v in c['values'] %}v{{loop.index}}{{ ", " if not loop.last }}{% endfor %});
}
{% endfor %}