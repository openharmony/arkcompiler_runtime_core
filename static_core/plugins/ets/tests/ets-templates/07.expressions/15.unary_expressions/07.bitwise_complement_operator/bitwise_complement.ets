{% for bw in bitwise_complement %}

/*---
desc: Bitwise complement operator
params: {{bw.type}} {{bw.origin}} => {{bw.dest}}
---*/

function main(): void {
  let a: {{bw.type}} = {{bw.origin}};
  assert(~a == {{bw.dest}});
}

{% endfor %}