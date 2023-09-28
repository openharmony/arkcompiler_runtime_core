{% for fc in float_comparison %}

/*---
desc: Integer comparison {{fc.xvalue}} {{fc.op}} {{fc.yvalue}}
---*/

function main(): void {
  let x: {{fc.xtype}} = {{fc.xvalue}};
  let y: {{fc.ytype}} = {{fc.yvalue}};
  assert (x {{fc.op}} y) == {{fc.result}};
}

{% endfor %}