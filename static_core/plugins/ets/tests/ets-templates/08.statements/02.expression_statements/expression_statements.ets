{% for item in expression_statements %}

/*---
desc: Expression statements
params: {{item.expr}}

---*/

function main(): void {
  {{item.expr}};
}

{% endfor %}