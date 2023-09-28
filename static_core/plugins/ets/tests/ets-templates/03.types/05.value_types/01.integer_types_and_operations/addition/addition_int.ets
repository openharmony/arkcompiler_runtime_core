{% for v in values %}
/*---
desc: check addition of two integers
---*/

function main(): void {
    const a: int = {{v.left}}
    const b: int = {{v.right}}
    assert (a + b) == {{v.result}}
}

{% endfor %}
