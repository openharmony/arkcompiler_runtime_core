/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

 // This test loads gc in an attempt 
 // to make old objects more likely to be garbage than young objects.

class TreeNode {
    public left : TreeNode | undefined;
    public right : TreeNode | undefined;
    public value : int;
}

class Random {
    public nextInt(n: int): int {
        return (random() * n) as int;
    }
}

class TestGCOld {
    private static size: int;
    private static workUnits: int;
    private static promoteRate: int;
    private static ptrMutRate: int;
    private static steps: int;

    private static MEG : int = 1000000;
    private static INSIGNIFICANT : int = 999;
    private static BYTES_PER_WORD : int = 4;
    private static BYTES_PER_NODE : int = 20;
    private static WORDS_DEAD : int = 100;

    private static treeHeight : int = 14;
    private static treeSize : long = TestGCOld.getBytesHeight(TestGCOld.treeHeight);

    private static youngBytes : long = 0;
    private static nodes : long = 0;
    private static actuallyMut : long = 0;
    private static mutatorSum : long = 0;
    private static aexport : int[] = [];

    private static trees : (TreeNode | undefined)[] = [undefined];
    private static where : int = 0;
    private static rnd = new Random();

    private static init() : void {
        let ntrees : int = ((TestGCOld.size * TestGCOld.MEG) / TestGCOld.treeSize) as int;
        TestGCOld.trees = new TreeNode[ntrees];

        console.println("Allocating " + ntrees + " trees.");
        console.println("  (" + (ntrees * TestGCOld.treeSize) + " bytes)");
        for (let i = 0; i < ntrees; i++) {
            TestGCOld.trees[i] = TestGCOld.getTree(TestGCOld.treeHeight);
        }
        console.println("  (" + TestGCOld.nodes + " nodes)");
    }

    private static getNodeHeight(h : int) : long {
        if (h == 0) {
            return 0;
        }
        else {
            let n : long = 1;
            while (h > 1) {
                n = n + n;
                h = h - 1;
            }
            return n + n - 1;
        }
    }

    private static getBytesHeight(h : int) : long {
        return TestGCOld.BYTES_PER_NODE * TestGCOld.getNodeHeight(h);
    }

    private static nodesToHeight(nodes : long) : int {
        let h : int = 1;
        let n : long = 1;
        while (n + n - 1 <= nodes) {
            n = n + n;
            h = h + 1;
        }
        return h - 1;
    }

    private static bytesToHeight(bytes : long) : int {
        return TestGCOld.nodesToHeight(bytes / TestGCOld.BYTES_PER_NODE);
    }

    private static getTree(h : int) : TreeNode | undefined {
        if (h == 0) return undefined;
        else {
            let res : TreeNode = new TreeNode();
            TestGCOld.nodes++;
            res.left = TestGCOld.getTree(h-1);
            res.right = TestGCOld.getTree(h-1);
            res.value = h;
            return res;
        }
    }

    private static executeStep(n : long) : void {
        let mutations : long = TestGCOld.actuallyMut;
        TestGCOld.doAllocateForYoungGen(n, TestGCOld.WORDS_DEAD);
        TestGCOld.runMutation(n);
        TestGCOld.doAllocateForOldGen(n / TestGCOld.promoteRate);
        TestGCOld.runMutationForOldGen(max(0, (mutations + TestGCOld.ptrMutRate) - TestGCOld.actuallyMut) as long);
    }

    private static doAllocateForYoungGen(n : long, nwords : int) : void {
        const nbytes : int = nwords*TestGCOld.BYTES_PER_WORD;
        let allocated : int = 0;
        while (allocated < n) {
            TestGCOld.aexport = new int[nwords];
            allocated += nbytes;
        }
        TestGCOld.youngBytes = TestGCOld.youngBytes + allocated;
    }

    private static runMutation(n : long) : void {
        let sum : int = 0;
        let limit : long = TestGCOld.workUnits*n/10;
        for (let k : long = 0; k < limit; k++) sum++;
        TestGCOld.mutatorSum = TestGCOld.mutatorSum + sum;
    }

    private static doAllocateForOldGen(n : long) : void {
        let full : int = (n / TestGCOld.treeSize) as int;
        let partial : long = n % TestGCOld.treeSize;

        for (let i : int = 0; i < full; i++) {
            TestGCOld.trees[TestGCOld.where++] = TestGCOld.getTree(TestGCOld.treeHeight);
            if (TestGCOld.where == TestGCOld.trees.length) TestGCOld.where = 0;
        }
        while (partial > TestGCOld.INSIGNIFICANT) {
            let h : int = TestGCOld.bytesToHeight(partial);
            let newTree : TreeNode | undefined = TestGCOld.getTree(h);
            TestGCOld.replaceTree(TestGCOld.trees[TestGCOld.where++]!, newTree!);
            if (TestGCOld.where == TestGCOld.trees.length) TestGCOld.where = 0;
            partial = partial - TestGCOld.getBytesHeight(h);
        }
    }

    private static runMutationForOldGen(n : long) : void {
        for (let i = 0; i < n/2; i++) {
            TestGCOld.swapSubtreesForOldGen();
        }
    }

    private static swapSubtreesForOldGen() : void {
        let index1 : int = TestGCOld.rnd.nextInt(TestGCOld.trees.length);
        let index2 : int = TestGCOld.rnd.nextInt(TestGCOld.trees.length);
        let depth : int = TestGCOld.rnd.nextInt(TestGCOld.treeHeight);
        let path : int = TestGCOld.rnd.nextInt(TestGCOld.treeHeight);
        let tn1  : TreeNode | undefined = TestGCOld.trees[index1];
        let tn2 : TreeNode | undefined = TestGCOld.trees[index2];
        for (let i = 0; i < depth; i++) {
            if ((path & 1) == 0) {
                tn1 = tn1!.left;
                tn2 = tn2!.left;
            } else {
                tn1 = tn1!.right;
                tn2 = tn2!.right;
            }
            path >>= 1;
        }
        let tmp : TreeNode | undefined;
        if ((path & 1) == 0) {
            tmp = tn1!.left;
            tn1!.left = tn2!.left;
            tn2!.left = tmp;
        } else {
            tmp = tn1!.right;
            tn1!.right = tn2!.right;
            tn2!.right = tmp;
        }
        TestGCOld.actuallyMut += 2;
    }

    private static doReplaceTreeWork(full : TreeNode, partial : TreeNode, dir : boolean) : void {
        let canGoLeft : boolean = full.left != undefined && full.left!.value > partial.value;
        let canGoRight : boolean = full.right != undefined && full.right!.value > partial.value;
        if (canGoLeft && canGoRight) {
            if (dir)
                TestGCOld.doReplaceTreeWork(full.left!, partial, !dir);
            else
                TestGCOld.doReplaceTreeWork(full.right!, partial, !dir);
        } else if (!canGoLeft && !canGoRight) {
            if (dir)
                full.left = partial;
            else
                full.right = partial;
            } else if (!canGoLeft) {
                full.left = partial;
                } else {
                full.right = partial;
                    }
    }

    private static replaceTree(full : TreeNode, partial : TreeNode) : void {
        let dir : boolean = (partial.value % 2) == 0;
        TestGCOld.actuallyMut++;
        TestGCOld.doReplaceTreeWork(full, partial, dir);
    }

    public static main(size: int, workUnits: int, promoteRate: int, ptrMutRate: int, steps: int): void {
        TestGCOld.size = size;
        TestGCOld.workUnits = workUnits;
        TestGCOld.promoteRate = promoteRate;
        TestGCOld.ptrMutRate = ptrMutRate;
        TestGCOld.steps = steps;

        console.println(TestGCOld.size + " megabytes of live storage");
        console.println(TestGCOld.workUnits + " work units per step");
        console.println("promotion ratio is 1:" + TestGCOld.promoteRate);
        console.println("pointer mutation rate is " + TestGCOld.ptrMutRate);
        console.println(TestGCOld.steps + " steps");

        TestGCOld.init();

        TestGCOld.youngBytes = 0;
        TestGCOld.nodes = 0;

        console.println("Initialization complete...");

        let start : long = Date.now() as long;

        for(let step = 0; step < TestGCOld.steps; step++) {
            TestGCOld.executeStep(TestGCOld.MEG);
        }

        let end : long = Date.now() as long;
        let secs : float = ((end-start)/1000.0) as float;
        
        console.println("\nTook " + secs + " sec in steady state.");
        console.println("Allocated " + TestGCOld.steps + " Mb of young gen garbage" 
                        + " (= " + (TestGCOld.steps/secs) as float + " Mb/sec)");
        console.println("    (actually allocated " + (TestGCOld.youngBytes/TestGCOld.MEG) as float + " megabytes)");
        let promoted : float = (TestGCOld.steps / TestGCOld.promoteRate) as float;
        console.println("Promoted " + promoted + 
                        " Mb (= " + (promoted/secs) as float + " Mb/sec)");
        console.println("    (actually promoted " + ((TestGCOld.nodes * TestGCOld.BYTES_PER_NODE)/TestGCOld.MEG) as float + " megabytes)");

        if (TestGCOld.ptrMutRate != 0) {
            console.println("Mutated " + TestGCOld.actuallyMut + " pointers (= " + TestGCOld.actuallyMut/secs + " ptrs/sec)");
        }

        console.println("Checksum = " + (TestGCOld.mutatorSum + TestGCOld.aexport.length));
    }
}

function main(): void {
    TestGCOld.main(50, 1, 20, 10, 1000);
    TestGCOld.main(50, 5, 20, 1, 500);
}
