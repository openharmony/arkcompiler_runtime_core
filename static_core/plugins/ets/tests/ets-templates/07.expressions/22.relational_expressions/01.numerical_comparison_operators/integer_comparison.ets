{% for ic in integer_comparison %}

/*---
desc: Integer comparison {{ic.xvalue}} {{ic.op}} {{ic.yvalue}}
---*/

function main(): void {
  let x: {{ic.xtype}} = {{ic.xvalue}};
  let y: {{ic.ytype}} = {{ic.yvalue}};
  assert (x {{ic.op}} y) == {{ic.result}};
}

{% endfor %}