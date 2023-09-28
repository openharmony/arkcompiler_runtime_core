{% for v in values %}
/*---
desc: check concatenation of string and unsigned byte
---*/

function main(): void {
    const a: {{v.left}}
    const b: {{v.right}}
    assert (a + b) == {{v.result}}
}

{% endfor %}
