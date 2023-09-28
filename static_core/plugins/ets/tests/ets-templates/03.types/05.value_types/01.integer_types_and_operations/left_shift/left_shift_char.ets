{% for v in values %}
/*---
desc: check left shift operation for char
---*/

function main(): void {
    const a: char = {{v.left}}
    const b: int = {{v.right}}
    assert (a << b) == {{v.result}}
}

{% endfor %}
