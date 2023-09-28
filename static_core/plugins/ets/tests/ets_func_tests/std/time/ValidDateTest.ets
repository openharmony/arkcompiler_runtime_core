const dates: long[] = [ -271821, -271820, 1, 1900, 1971, 275759, 275760, 275761];
const check: boolean[] = [false, true, true, true, true, true, true, false];

function main(): int {
    for (let i: int; i< dates.length; i++) {
        let date: Date = new Date(dates[i], 0);
        let dateComplex = new Date(dates[i], 0);
        let vd = date.isDateValid();
        let vdComplex = dateComplex.isDateValid();
        if ((vd || vdComplex) != check[i]) return 1;
    }
    return 0;
}
