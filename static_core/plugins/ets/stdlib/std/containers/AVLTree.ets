/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

package std.containers;

/**
 * Node for a tree
 */
class Node<K extends Comparable<Object>, V> {
    left: Node<K, V> | null;
    key: K;
    value: V;
    height: int;
    right: Node<K, V> | null;
    constructor(left: Node<K, V> | null, key: K, value: V, height: int, right: Node<K, V> | null) {
        this.key = key;
        this.value = value;
        this.left = left;
        this.right = right;
        this.height = height;
    }
}

/**
 * Constructs new empty Node
 *
 * @returns empty node
 */
export function newEmpty<K extends Comparable<Object>, V>(): Node<K, V> | null {
    return null;
}

/**
 * Checks if Node is empty
 *
 * @param n
 *
 * @returns true if Node is empty, false otherwise
 */
export function isEmpty<K extends Comparable<Object>, V>(n: Node<K, V> | null): boolean {
    return n == null;
}

/**
 * Checks if Node is a leaf
 *
 * @param n
 *
 * @returns true if Node is a leaf, false otherwise
 */
export function isLeaf<K extends Comparable<Object>, V>(n: Node<K, V> | null): boolean {
    return n != null && n!.left == null && n!.right == null;
}

/**
 * Checks if Node is a leaf
 *
 * @param n
 *
 * @returns true if Node is a leaf, false otherwise
 */
export function isNode<K extends Comparable<Object>, V>(n: Node<K, V> | null): boolean {
    return n != null && (n!.left != null || n!.right != null);
}

export function height<K extends Comparable<Object>, V>(node: Node<K, V> | null): int {
    if (isEmpty<K, V>(node)) {
        return 0;
    } else if (isLeaf(node)) {
        return 1;
    } else {
        return node!.height;
    }
}

export function newNode<K extends Comparable<Object>, V>(left: Node<K, V> | null, k: K, v: V, height: int, right: Node<K, V> | null): Node<K, V> {
    return new Node<K, V>(left, k, v, height, right);
}

export function newLeaf<K extends Comparable<Object>, V>(k: K, v: V): Node<K, V> {
    return new Node<K, V>(newEmpty<K, V>(), k, v, 1, newEmpty<K, V>());
}

function legalLeftKey<K extends Comparable<Object>, V>(node: Node<K, V> | null, key: K): void {
    if (isEmpty(node)) {
        return;
    }

    let leftKey: K = node!.key;
    assertLT(leftKey.compareTo(key), 0);
}

function legalRightKey<K extends Comparable<Object>, V>(node: Node<K, V> | null, key: K): void {
    if (isEmpty(node)) {
        return;
    }

    let rightKey: K = node!.key ;
    assertLT(0, rightKey.compareTo(key));
}

function inv<K extends Comparable<Object>, V>(node: Node<K, V> | null): void {
    if (isNode(node)) {
        let hl = height(node!.left);
        let hr = height(node!.right);

        inv(node!.left);
        inv(node!.right);

        legalLeftKey(node!.left, node!.key);
        legalRightKey(node!.right, node!.key);

        assertEQ(node!.height, max(hl, hr) + 1);
        assertLE(abs(hl - hr), 2);
    }
}


function updateHeight<K extends Comparable<Object>, V>(node: Node<K, V> | null): void {
    if (isNode(node)) {
        let newHeight = max(height(node!.left), height(node!.right)) + 1;
        if (newHeight != node!.height) {
            node!.height = newHeight;
        }
    } else if (isLeaf(node)) {
        node!.height = 1;
    } else {
        throw new AssertionError("This line should be unreachable!")
    }
}

function balanceTree<K extends Comparable<Object>, V>(node: Node<K, V> | null): Node<K, V> | null {
    if (isEmpty(node) || isLeaf(node)) {
        return node;
    } else if (isNode(node)) {
        let rootNode = node!;
        let hl = height(rootNode.left);
        let hr = height(rootNode.right);

        if (hl > hr + 2) {
            assertTrue(!(isEmpty(rootNode.left)), "rootNode.left is empty!");
            assertTrue(!(isLeaf(rootNode.left)), "rootNode.left is leaf!");
            let leftNode = rootNode.left!;
            if (height(leftNode.left) >= height(leftNode.right)) {
                rootNode.left = leftNode.right;
                leftNode.right = rootNode;

                updateHeight(rootNode);
                updateHeight(leftNode);

                return leftNode;
            } else {
                assertTrue(!(isEmpty(leftNode.right)), "leftNode.right is empty!");
                assertTrue(!(isLeaf(leftNode.right)), "leftNode.right is leaf!");
                let lrNode = leftNode.right!;

                leftNode.right = lrNode.left;
                rootNode.left = lrNode.right;
                lrNode.right = rootNode;
                lrNode.left = leftNode;

                updateHeight(leftNode);
                updateHeight(rootNode);
                updateHeight(lrNode);

                return lrNode;
            }
        } else if (hr > hl + 2) {
            assertTrue(!(isEmpty(rootNode.right)), "rootNode.right is empty!");
            assertTrue(!(isLeaf(rootNode.right)), "rootNode.right is leaf!");
            let rightNode = rootNode.right!;
            if (height(rightNode.right) >= height(rightNode.left)) {
                rootNode.right = rightNode.left;
                rightNode.left = rootNode;

                updateHeight(rootNode);
                updateHeight(rightNode);

                return rightNode;
            } else {
                assertTrue(!(isEmpty(rightNode.left)), "rightNode.left is empty!")
                assertTrue(!(isLeaf(rightNode.left)), "rightNode.left is leaf!");
                let rlNode = rightNode.left!;

                rightNode.left = rlNode.right;
                rootNode.right = rlNode.left;
                rlNode.left = rootNode;
                rlNode.right = rightNode;

                updateHeight(rightNode);
                updateHeight(rootNode);
                updateHeight(rlNode);

                return rlNode;
            }
        } else {
            updateHeight(node);
            return node;
        }
    } else {
        throw new AssertionError("This line should be unreachable!");
    }
}

function setLeft<K extends Comparable<Object>, V>(node: Node<K, V>, tree: Node<K, V> | null): void {
    let tree1 = balanceTree(tree);
    assertTrue(isNode(node), "Value " + node + " is not a node!");

    let n = node;
    if (n.left != tree1) {
        n.left = tree1;
    }

    updateHeight(n);
}

function setRight<K extends Comparable<Object>, V>(node: Node<K, V>, tree: Node<K, V> | null): void {
    let tree1 = balanceTree(tree);
    assertTrue(isNode(node), "Value " + node + " is not a node!");

    let n = node;
    if (n.right != tree1) {
        n.right = tree1;
    }

    updateHeight(n);
}
class addResult<K extends Comparable<Object>, V> {
    t: Node<K, V>;
    res: boolean;
    constructor(t: Node<K, V>, res: boolean) {
        this.t = t;
        this.res = res;
    }
}

function add<K extends Comparable<Object>, V>(node: Node<K, V> | null, k: K, v: V, replace: boolean): addResult<K, V> {
    if (isEmpty(node)) {
        return new addResult<K, V>(newLeaf(k, v), true);
    }

    if (isLeaf(node)) {
        let leaf = node as Node<K, V>;
        let c = leaf.key.compareTo(k);
        if (c == 0) {
            if (replace) {
                leaf.value = v;
                return new addResult<K, V>(leaf, false);
            } else {
                return new addResult<K, V>(leaf, false);
            }
        } else {
            if (c < 0) {
                return new addResult<K, V>(newNode(node, k, v, 2, newEmpty<K, V>()), true);
            } else {
                return new addResult<K, V>(newNode(newEmpty<K, V>(), k, v, 2, node), true);
            }
        }
    }

    if (isNode(node)) {
        let nnode = node as Node<K, V>;
        let c = k.compareTo(nnode.key);
        if (c == 0) {
            if (replace) {
                nnode.value = v;
            }
            return new addResult<K, V>(nnode, false);
        } else {
            if (c < 0) {
                let r: addResult<K, V> = add(nnode.left, k, v, replace);
                setLeft(nnode, r.t);
                return new addResult<K, V>(nnode, r.res);
            } else {
                let r: addResult<K, V> = add(nnode.right, k, v, replace);
                setRight(nnode, r.t);
                return new addResult<K, V>(nnode, r.res);
            }
        }
    }

    throw new Error("Invalid node");
}


export function addToTree<K extends Comparable<Object>, V>(node: Node<K, V> | null, k: K, v: V): Node<K, V> {
    let r: addResult<K, V> = add(node, k, v, true);
    if (!r.res) {
        return balanceTree(r.t) as Node<K, V>
    } else {
        return r.t
    }
}

function minElt<K extends Comparable<Object>, V>(node: Node<K, V> | null): Node<K, V> | null {
    if (isEmpty(node) || isLeaf(node)) {
        return node;
    }
    if (isEmpty(node!.left)) {
        return node;
    } else {
        return minElt(node!.left);
    }
}

function removeMinElt<K extends Comparable<Object>, V>(node: Node<K, V>): Node<K, V> | null {
    assertTrue(!(isEmpty(node)), "Tree " + node + " is empty!");
    if (isLeaf(node)) {
        return null;
    }
    if (isEmpty(node.left)) {
        return node.right;
    } else if (isLeaf(node.left) && isEmpty(node.right)) {
        return newLeaf(node.key, node.value);
    } else if (isLeaf(node.left)) {
        setLeft(node, null);
        return node;
    } else {
        setLeft(node, removeMinElt(node.left as Node<K, V>));
        return node;
    }
}

export function mergeTree<K extends Comparable<Object>, V>(t1: Node<K, V> | null, t2: Node<K, V> | null): Node<K, V> | null {
    if (isEmpty(t1)) {
        return t2;
    }
    if (!isEmpty(t1) && isEmpty(t2)) {
        return t1;
    }
    let nt2 = t2 as Node<K, V>;

    let tree: Node<K, V> = minElt(nt2) as Node<K, V>;

    assertTrue(!(isEmpty(tree)), "Tree " + tree + " is empty!");

    if (isLeaf(tree)) {
        let leaf = tree;
        let t3 = balanceTree(removeMinElt(nt2));
        return newNode(t1, leaf.key, leaf.value, max(height(t1), height(t3)) + 1, t3);
    } else {
        let node = tree;
        setRight(node, removeMinElt(nt2));
        setLeft(node, t1);
        return node;
    }
}

final class removeResult<K extends Comparable<Object>, V> {
    t: Node<K, V> | null;
    res: boolean;
    constructor(t: Node<K, V> | null, res: boolean) {
        this.t = t;
        this.res = res;
    }
}

function remove<K extends Comparable<Object>, V>(node: Node<K, V> | null, k: K): removeResult<K, V> {
    if (isEmpty(node)) {
        return new removeResult<K, V>(node, false);
    } else if (isLeaf(node)) {
        let leaf = node!;
        if (leaf.key.compareTo(k) == 0) {
            return new removeResult<K, V>(newEmpty<K, V>(), true);
        } else {
            return new removeResult<K, V>(node, false);
        }
    } else {
        let nnode = node as Node<K, V>;
        let c = k.compareTo(nnode.key);
        if (c == 0) {
            return new removeResult<K, V>(mergeTree(nnode.left, nnode.right), true);
        } else if (c < 0) {
            let r: removeResult<K, V> = remove(nnode.left, k);
            setLeft(nnode, r.t)
            return new removeResult<K, V>(nnode, r.res);
        } else {
            let r: removeResult<K, V> = remove(nnode.right, k);
            setRight(nnode, r.t)
            return new removeResult<K, V>(nnode, r.res);
        }
    }
}

export function removeFromTree<K extends Comparable<Object>, V>(node: Node<K, V> | null, k: K): Node<K, V> | null {
    let r: removeResult<K, V> = remove(node, k);
    return balanceTree(r.t);
}

/**
 * Finds the specified value in the Node tree
 *
 * @param node tree root
 *
 * @param k value to find
 *
 * @returns value
 * TODO: replace null
 */
export function lookupTree<K extends Comparable<Object>, V>(node: Node<K, V> | null, k: K): V | null {
    if (isEmpty(node)) {
        return null;
    } else if (isLeaf(node)) {
        let leaf = node!;
        if (k.compareTo(leaf.key) == 0) {
            return leaf.value;
        } else {
            return null;
        }
    } else {
        let c = k.compareTo(node!.key);
        if (c == 0) {
            return node!.value;
        } else if (c < 0) {
            return lookupTree(node!.left, k);
        } else {
            return lookupTree(node!.right, k);
        }
    }
}

export function count<K extends Comparable<Object>, V>(node: Node<K, V> | null): int {
    if (isEmpty(node)) {
        return 0;
    } else if (isLeaf(node)) {
        return 1;
    } else {
        return 1 + count(node!.left) + count(node!.right);
    }
}
