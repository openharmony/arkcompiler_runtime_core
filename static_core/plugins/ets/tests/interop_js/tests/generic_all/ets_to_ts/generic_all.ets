/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

export type GenericArray<T> = T[];

export interface GenericPair<K, V> {
  key: K;
  value: V;
}

export function fooIdentity<T>(arg: T): T {
  return arg;
}

export function fooMultiParam<T, V>(arg1: T, arg2: V): T {
  return arg1;
}

export function fooUnionReturn<T extends Object, U extends Object>(obj1: T, obj2: U): T | U {
  let r: T | U = obj1;
  return r;
}

export function fooDefault<T = string>(arg: T): T {
  return arg;
}

export function fooNullable<T>(arg: T | undefined): T | undefined {
  return arg;
}

export class NClass {
  public data: string;
  constructor(data: string) {
    this.data = data;
  }
}

export class Base {
  bar(): void {}
}

export class Child extends Base {
  foo(): void {}
}

export class GClass<T> {
  private data: T;
  public static staticData: string = 'STATICDATA';
  constructor(data: T) {
    this.data = data;
  }
  getData(): T {
    return this.data;
  }
  setData(d: T): void {
    this.data = d;
  }
  transfer<U>(mapper: (val: T) => U): U {
    return mapper(this.data);
  }
  static print(): void {}
}

export class GClassDefault<T = string> {
  public data: T;
  constructor(data: T) {
    this.data = data;
  }
}

export class GExNClass<T> extends NClass {
  private dataT: T;
  constructor(dataT: T) {
    super('NClass' + typeof dataT);
    this.dataT = dataT;
  }
  setData(arg: T): void {
    this.dataT = arg;
  }
  getData(): T {
    return this.dataT;
  }
}

export class GExGClass<T, V> extends GClass<V> {
  private dataT: T;
  constructor(arg1: T, arg2: V) {
    super(arg2);
    this.dataT = arg1;
  }
  setDataNew(arg1: T, arg2: V): void {
    super.setData(arg2);
    this.dataT = arg1;
  }
  getDataT(): T {
    return this.dataT;
  }
}

export interface GInterface<T> {
  dataT?: T;
}

export interface NInterface {
  data?: string;
}

export class GImClass<T, V> implements GInterface<V> {
  public dataT?: V;
  public dataV?: T;
}

export class GImNClass<T, V> implements NInterface {
  public data?: string;
  public dataT?: V;
  public dataV?: T;
}

export class ToExClass<T> {
  private data?: T;
  constructor(data?: T) {
    this.data = data;
  }
  getData(): T | undefined {
    return this.data;
  }
  setData(d: T): void {
    this.data = d;
  }
}

export class GClassExtend<T extends ToExClass<T>> {
  public data?: T;
}

export function fooExtends<T extends ToExClass<T>>(arg: T): T {
  return arg;
}

export class GenericCircle<T extends GenericCircle<T>> {
  public data?: T;
}

export class DynExtend extends ToExClass<DynExtend> {
  public dataT: string;
  constructor(dataT: string) {
    super();
    this.dataT = dataT;
  }
}

export class DynCircle extends GenericCircle<DynCircle> {
  public radius: number = 10;
}

export class GNullableClass<T> {
  public data: T | undefined;
  constructor(data?: T) {
    this.data = data;
  }
  getData(): T | undefined {
    return this.data;
  }
  setData(data: T): void {
    this.data = data;
  }
  hasData(): boolean {
    return this.data !== undefined;
  }
}

export class GPairClass<T, U> {
  private first: T;
  private second: U;
  constructor(first: T, second: U) {
    this.first = first;
    this.second = second;
  }
  getFirst(): T {
    return this.first;
  }
  getSecond(): U {
    return this.second;
  }
  setFirst(first: T): void {
    this.first = first;
  }
  setSecond(second: U): void {
    this.second = second;
  }
}

export class GMethodClass<T> {
  private data: T;
  constructor(data: T) {
    this.data = data;
  }
  process<U>(mapper: (val: T) => U): U {
    return mapper(this.data);
  }
}

export interface GMethodInterface<T> {
  getValue(): T;
  setValue(val: T): void;
}

export class GMethodImplClass<T> implements GMethodInterface<T> {
  private value: T;
  constructor(value: T) {
    this.value = value;
  }
  getValue(): T {
    return this.value;
  }
  setValue(val: T): void {
    this.value = val;
  }
}

export class Animal {
  public name: string = 'Animal';
}

export class Dog extends Animal {
  public name: string = 'Dog';
}

export class Cat extends Animal {
  public name: string = 'Cat';
}

export class AnimalWithAge extends Animal {
  public age: number = 5;
}

export class AgedDog extends Dog {
  public age: number = 3;
}

export class GenericContainer<T> {
  private item: T;
  constructor(item: T) {
    this.item = item;
  }
  getItem(): T {
    return this.item;
  }
  setItem(item: T): void {
    this.item = item;
  }
}

export function createAnimalContainer(): GenericContainer<Animal> {
  return new GenericContainer<Animal>(new Animal());
}

export function createDogContainer(): GenericContainer<Dog> {
  return new GenericContainer<Dog>(new Dog());
}

export class SimpleConsumer<T> {
  public consumedItem: T | undefined;
  consume(item: T): void {
    this.consumedItem = item;
  }
  getConsumed(): T | undefined {
    return this.consumedItem;
  }
  hasConsumed(): boolean {
    return this.consumedItem !== undefined;
  }
}

export function processWithSimpleConsumer<T>(consumer: SimpleConsumer<T>, item: T): void {
  consumer.consume(item);
}

export class SimpleProducer<T> {
  private item: T;
  constructor(item: T) {
    this.item = item;
  }
  produce(): T {
    return this.item;
  }
}

export function getFromSimpleProducer<T>(producer: SimpleProducer<T>): T {
  return producer.produce();
}

export class InvariantHolder<T> {
  private data: T;
  constructor(data: T) {
    this.data = data;
  }
  getData(): T {
    return this.data;
  }
  setData(data: T): void {
    this.data = data;
  }
  exchange(other: InvariantHolder<T>): void {
    let temp: T = this.data;
    this.data = other.getData();
    other.setData(temp);
  }
}

export function testInvariance(holder: InvariantHolder<string>): string {
  return holder.getData();
}

export class CovariantReader<T extends Animal> {
  private animal: T;
  constructor(animal: T) {
    this.animal = animal;
  }
  readName(): string {
    return this.animal.name;
  }
}

export function readAnimalName(reader: CovariantReader<Animal>): string {
  return reader.readName();
}

export function readDogName(reader: CovariantReader<Dog>): string {
  return reader.readName();
}

export class ContravariantWriter<T extends Animal> {
  public writtenName: string = '';
  write(animal: T): void {
    this.writtenName = animal.name;
  }
  getWrittenName(): string {
    return this.writtenName;
  }
}

export function writeToAnimal(writer: ContravariantWriter<Animal>): void {
  writer.write(new Animal());
}

export function writeToDog(writer: ContravariantWriter<Dog>): void {
  writer.write(new Dog());
}

export interface HasAge {
  age: number;
}

export class BoundedGenericClass<T extends AnimalWithAge> {
  private item: T;
  constructor(item: T) {
    this.item = item;
  }
  getItem(): T {
    return this.item;
  }
  getItemName(): string {
    return this.item.name;
  }
  getItemAge(): number {
    return this.item.age;
  }
}

export function createBoundedClass(): BoundedGenericClass<AnimalWithAge> {
  return new BoundedGenericClass<AnimalWithAge>(new AnimalWithAge());
}

export function getBoundedItemName(holder: BoundedGenericClass<AnimalWithAge>): string {
  return holder.getItemName();
}

export function getBoundedItemAge(holder: BoundedGenericClass<AnimalWithAge>): number {
  return holder.getItemAge();
}

export class NestedGenericOuter<T> {
  private inner: NestedGenericInner<T>;
  constructor(value: T) {
    this.inner = new NestedGenericInner<T>(value);
  }
  getInner(): NestedGenericInner<T> {
    return this.inner;
  }
  getValue(): T {
    return this.inner.getValue();
  }
}

export class NestedGenericInner<T> {
  private value: T;
  constructor(value: T) {
    this.value = value;
  }
  getValue(): T {
    return this.value;
  }
  setValue(value: T): void {
    this.value = value;
  }
}

export function createNestedGeneric(): NestedGenericOuter<string> {
  return new NestedGenericOuter<string>('nested');
}

export function getNestedValue(outer: NestedGenericOuter<string>): string {
  return outer.getValue();
}

export class MultiTypeConstraint<T extends Animal, U extends AgedDog> {
  private animal: T;
  private aged: U;
  constructor(animal: T, aged: U) {
    this.animal = animal;
    this.aged = aged;
  }
  getAnimalName(): string {
    return this.animal.name;
  }
  getAgedAge(): number {
    return this.aged.age;
  }
}

export function createMultiConstraint(): MultiTypeConstraint<Dog, AgedDog> {
  return new MultiTypeConstraint<Dog, AgedDog>(new Dog(), new AgedDog());
}

export function getMultiConstraintAnimalName(holder: MultiTypeConstraint<Dog, AgedDog>): string {
  return holder.getAnimalName();
}

export function getMultiConstraintAge(holder: MultiTypeConstraint<Dog, AgedDog>): number {
  return holder.getAgedAge();
}

export class TypeDependency<T, S extends T> {
  private first: T;
  private second: S;
  constructor(first: T, second: S) {
    this.first = first;
    this.second = second;
  }
  getFirst(): T {
    return this.first;
  }
  getSecond(): S {
    return this.second;
  }
  getSecondAsFirst(): T {
    return this.second;
  }
  setFirst(first: T): void {
    this.first = first;
  }
  setSecond(second: S): void {
    this.second = second;
  }
}

export function createTypeDependency(): TypeDependency<Animal, Dog> {
  return new TypeDependency<Animal, Dog>(new Animal(), new Dog());
}

export function getTypeDependencyFirst(holder: TypeDependency<Animal, Dog>): string {
  return holder.getFirst().name;
}

export function getTypeDependencySecond(holder: TypeDependency<Animal, Dog>): string {
  return holder.getSecond().name;
}

export class MultiDefault<T1, T2 = number, T3 = string> {
  public value1: T1;
  public value2: T2;
  public value3: T3;
  constructor(v1: T1, v2: T2, v3: T3) {
    this.value1 = v1;
    this.value2 = v2;
    this.value3 = v3;
  }
  getValue1(): T1 {
    return this.value1;
  }
  getValue2(): T2 {
    return this.value2;
  }
  getValue3(): T3 {
    return this.value3;
  }
}

export function createMultiDefaultFull(): MultiDefault<boolean, number, string> {
  return new MultiDefault<boolean, number, string>(true, 42, 'test');
}

export function createMultiDefaultPartial(): MultiDefault<boolean> {
  return new MultiDefault<boolean, number, string>(true, 100, 'partial');
}

export function getMultiDefaultValues(holder: MultiDefault<boolean>): string {
  return holder.getValue1() + '-' + holder.getValue2() + '-' + holder.getValue3();
}

export class UnionConstraint<T extends Animal | NClass> {
  private item: T;
  constructor(item: T) {
    this.item = item;
  }
  getItem(): T {
    return this.item;
  }
}

export function createUnionConstraintAnimal(): UnionConstraint<Animal> {
  return new UnionConstraint<Animal>(new Animal());
}

export function createUnionConstraintNClass(): UnionConstraint<NClass> {
  return new UnionConstraint<NClass>(new NClass('unionNClass'));
}

export function getUnionConstraintAnimal(holder: UnionConstraint<Animal>): Animal {
  return holder.getItem();
}

export function getUnionConstraintNClass(holder: UnionConstraint<NClass>): NClass {
  return holder.getItem();
}

export class GenericMethodDefault<T> {
  private data: T;
  constructor(data: T) {
    this.data = data;
  }
  getData(): T {
    return this.data;
  }
  processData<U = T>(mapper: (val: T) => U): U {
    return mapper(this.data);
  }
  processWithClassDefault(): T {
    return this.data;
  }
}

export function createGenericMethodDefault(): GenericMethodDefault<string> {
  return new GenericMethodDefault<string>('defaultMethod');
}

export function testGenericMethodDefault(holder: GenericMethodDefault<string>): string {
  return holder.processWithClassDefault();
}

export function testGenericMethodExplicit(holder: GenericMethodDefault<string>): number {
  return holder.processData<number>((val: string) => val.length);
}

export class GenericRecursive<T extends GenericRecursive<T>> {
  public self?: T;
  public data?: string;
  constructor(data?: string) {
    this.data = data;
  }
  getSelf(): T | undefined {
    return this.self;
  }
  getData(): string | undefined {
    return this.data;
  }
  setSelf(s: T): void {
    this.self = s;
  }
}

export class ConcreteRecursive extends GenericRecursive<ConcreteRecursive> {
  public value: number = 100;
  constructor(data: string, value: number) {
    super(data);
    this.value = value;
  }
}

export function createConcreteRecursive(): ConcreteRecursive {
  let cr = new ConcreteRecursive('recursive', 200);
  cr.self = cr;
  return cr;
}

export function getRecursiveData(holder: ConcreteRecursive): string {
  return holder?.getData()!;
}

export function getRecursiveSelfValue(holder: ConcreteRecursive): number {
  return holder?.getSelf()!.value;
}

export class GenericFunctionType<T> {
  private callback: (arg: T) => T;
  constructor(cb: (arg: T) => T) {
    this.callback = cb;
  }
  execute(arg: T): T {
    return this.callback(arg);
  }
  getCallback(): (arg: T) => T {
    return this.callback;
  }
}

export function createGenericFunctionType(): GenericFunctionType<string> {
  return new GenericFunctionType<string>((s: string) => s.toUpperCase());
}

export function executeGenericFunction(holder: GenericFunctionType<string>, arg: string): string {
  return holder.execute(arg);
}

export class GenericWithFunctionParam<T, U> {
  private transformer: (arg: T) => U;
  private input: T;
  constructor(input: T, transformer: (arg: T) => U) {
    this.input = input;
    this.transformer = transformer;
  }
  transform(): U {
    return this.transformer(this.input);
  }
  getInput(): T {
    return this.input;
  }
  getTransformer(): (arg: T) => U {
    return this.transformer;
  }
}

export function createGenericWithFunctionParam(): GenericWithFunctionParam<string, number> {
  return new GenericWithFunctionParam<string, number>('hello', (s: string) => s.length);
}

export function testGenericFunctionParam(holder: GenericWithFunctionParam<string, number>): number {
  return holder.transform();
}

export class KeyOfTestClass {
  public field1: number = 1;
  public field2: string = 'field2';
}

export class KeyOfGenericClass<T extends keyof KeyOfTestClass> {
  private key: T;
  constructor(key: T) {
    this.key = key;
  }
  getKey(): T {
    return this.key;
  }
}

export function createKeyOfGenericField1(): KeyOfGenericClass<'field1'> {
  return new KeyOfGenericClass<'field1'>('field1');
}

export function createKeyOfGenericField2(): KeyOfGenericClass<'field2'> {
  return new KeyOfGenericClass<'field2'>('field2');
}

export function getKeyOfValue(holder: KeyOfGenericClass<'field1' | 'field2'>): string {
  return holder.getKey();
}

export let dynVal: number = 1;
export let dynObject: NClass = new NClass('dyn');
export let nClassString: NClass = new NClass('string');
export let genericArrayInt: GenericArray<number> = [1, 2, 3];
export let genericArrayString: GenericArray<string> = ['a', 'b', 'c'];

export function createGenericPair(): GenericPair<string, number> {
  return { key: 'age', value: 30 };
}

export function getGenericPairKey(pair: GenericPair<string, number>): string {
  return pair.key;
}

export function getGenericPairValue(pair: GenericPair<string, number>): number {
  return pair.value;
}

export interface PartialInterface {
  title: string;
  desc: string;
}

export interface ReadonlyInterface {
  id: number;
  name: string;
}

export interface RequiredInterface {
  title?: string;
  desc?: string;
}

export class RecordTestClass {
  public data: string = 'test';
}

export function createPartialObject(): Partial<PartialInterface> {
  return { title: 'testTitle' };
}

export function getPartialTitle(obj: Partial<PartialInterface>): string | undefined {
  return obj.title;
}

export function createReadonlyObject(): Readonly<ReadonlyInterface> {
  return { id: 1, name: 'readonlyName' };
}

export function getReadonlyId(obj: Readonly<ReadonlyInterface>): number {
  return obj.id;
}

export function getReadonlyName(obj: Readonly<ReadonlyInterface>): string {
  return obj.name;
}

export function createRequiredObject(): Required<RequiredInterface> {
  return { title: 'reqTitle', desc: 'reqDesc' };
}

export function getRequiredTitle(obj: Required<RequiredInterface>): string {
  return obj.title!;
}

export function getRequiredDesc(obj: Required<RequiredInterface>): string {
  return obj.desc!;
}

export function createNonNullableValue(): NonNullable<string | null> {
  return 'nonNullValue';
}

export function getNonNullableLength(val: NonNullable<string | null>): number {
  return val.length;
}