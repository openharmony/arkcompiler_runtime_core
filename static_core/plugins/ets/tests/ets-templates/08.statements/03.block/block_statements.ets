{% for item in block_statements %}

/*---
desc: Expression statements
params: >
  {{item.expr}}

---*/

function main(): void {
  {
    {{item.expr}};
  }
}

{% endfor %}