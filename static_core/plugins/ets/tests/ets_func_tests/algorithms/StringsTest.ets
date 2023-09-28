
function is_palindrom(text: String): boolean {
    let halve: int = text.length() / 2;
    for (let i: int = 0; i < halve; i++) {
        if (text.charAt(i) != text.charAt(text.length() - i - 1)) {
            return false;
        }
    }
    return true;
}

function random_string(len: int): String {
    const t = "qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM";
    const l = t.length();
    let sb = new StringBuilder();
    for (let i = 0; i < len; i++) {
        sb.append(t.charAt((random()*l) as int));
    }
    return sb.toString();
}

function test(result: boolean, name: String): int {
    if (result) {
        console.println("PASSED: " + name);
        return 0;
    }
    console.println("FAILED: " + name);
    return 1;
}

function main(): int {
    let failures: int = 0;
    failures += test(is_palindrom("ABBA"), "ABBA is a palindrom");
    failures += test(is_palindrom("KABAK"), "KABAK is a palindrom");
    failures += test(!is_palindrom("ABbA"), "ABbA is not a palindrom");
    for (let i=0; i<5; i++) {
        let word = "a" + random_string(60) + "z";
        failures += test(!is_palindrom(word), word + " is not a palindrom");
    }
    return failures;
}
