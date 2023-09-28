{% for c in cases %}
/*---
desc: String contexts conversion.
---*/
{{c.defs}}

function main(): int {
  {%- for v in c['val'] %}
  let v{{loop.index}}{{": " + v[0]}} = {{v[1]}};
  {%- endfor %}

  let res = "" + {% for v in c['val'] %}v{{loop.index}}{{" + " if not loop.last}}{% endfor %};
  let expected = {% for v in c['val'] %}{{v[2]}}{{" + " if not loop.last}}{% endfor %};

  if (res.equals(expected)) return 0;
  return 1;
}
{% endfor %}
