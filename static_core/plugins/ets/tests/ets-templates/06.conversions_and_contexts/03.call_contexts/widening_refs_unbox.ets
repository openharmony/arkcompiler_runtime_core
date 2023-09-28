{% for case in cases %}
/*---
desc: Invocation contexts allow the use a widening reference conversion followed by an unboxing conversion.
params: {{case.val | safe}} to {{case.type}}
---*/

class B {
  {%- for v in case['ptype'] %}
  fld1_{{v}}: {{v}};
  fld2_{{v}}: {{v}};
  {%- endfor %}

  constructor<T extends {{case.type}}>({% for v in case['ptype'] %}p{{loop.index}}: T{{ ", " if not loop.last }}{% endfor %}) {
    {%- for v in case['ptype'] %}
    this.fld1_{{v}} = p{{loop.index}};
    {%- endfor %}
  }

  public foo<T extends {{case.type}}>({% for v in case['ptype'] %}p{{loop.index}}: T{{ ", " if not loop.last }}{% endfor %}): int {
    {%- for v in case['ptype'] %}
    this.fld2_{{v}} = p{{loop.index}};
    {%- endfor %}
    if (
      {%- for v in case['ptype'] %}
      this.fld1_{{v}} != this.fld2_{{v}}{{ " ||" if not loop.last }}
      {%- endfor %}
    ) {
      return 1;
    } else {
      return 0;
    }
  }
}

function main(): int {
  let b: B = new B({% for v in case['ptype'] %}{{case.val | safe}}{{ ", " if not loop.last }}{% endfor %});
  return b.foo({% for v in case['ptype'] %}{{case.val | safe}}{{ ", " if not loop.last }}{% endfor %});
}
{% endfor %}
