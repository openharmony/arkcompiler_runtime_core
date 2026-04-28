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

const etsVm = globalThis.gtest.etsVm;

const fooIdentity = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'fooIdentity');
const fooMultiParam = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'fooMultiParam');
const fooUnionReturn = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'fooUnionReturn');
const fooDefault = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'fooDefault');
const fooNullable = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'fooNullable');
const fooExtends = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'fooExtends');

const NClass = etsVm.getClass('Ltest_generic_all/NClass;');
const Base = etsVm.getClass('Ltest_generic_all/Base;');
const Child = etsVm.getClass('Ltest_generic_all/Child;');
const GClass = etsVm.getClass('Ltest_generic_all/GClass;');
const GExNClass = etsVm.getClass('Ltest_generic_all/GExNClass;');
const GExGClass = etsVm.getClass('Ltest_generic_all/GExGClass;');
const DGExNClass = etsVm.getClass('Ltest_generic_all/DGExNClass;');
const DGExGClass = etsVm.getClass('Ltest_generic_all/DGExGClass;');
const GClassDefault = etsVm.getClass('Ltest_generic_all/GClassDefault;');
const ToExClass = etsVm.getClass('Ltest_generic_all/ToExClass;');
const GClassExtend = etsVm.getClass('Ltest_generic_all/GClassExtend;');
const GenericCircle = etsVm.getClass('Ltest_generic_all/GenericCircle;');
const GImClass = etsVm.getClass('Ltest_generic_all/GImClass;');
const GImNClass = etsVm.getClass('Ltest_generic_all/GImNClass;');
const GImDClass = etsVm.getClass('Ltest_generic_all/GImDClass;');
const GNullableClass = etsVm.getClass('Ltest_generic_all/GNullableClass;');
const GPairClass = etsVm.getClass('Ltest_generic_all/GPairClass;');
const GMethodClass = etsVm.getClass('Ltest_generic_all/GMethodClass;');
const GMethodImplClass = etsVm.getClass('Ltest_generic_all/GMethodImplClass;');

const EtsAnimal = etsVm.getClass('Ltest_generic_all/Animal;');
const EtsDog = etsVm.getClass('Ltest_generic_all/Dog;');
const EtsCat = etsVm.getClass('Ltest_generic_all/Cat;');
const AnimalWithAge = etsVm.getClass('Ltest_generic_all/AnimalWithAge;');
const AgedDog = etsVm.getClass('Ltest_generic_all/AgedDog;');
const GenericContainer = etsVm.getClass('Ltest_generic_all/GenericContainer;');
const SimpleConsumer = etsVm.getClass('Ltest_generic_all/SimpleConsumer;');
const SimpleProducer = etsVm.getClass('Ltest_generic_all/SimpleProducer;');
const InvariantHolder = etsVm.getClass('Ltest_generic_all/InvariantHolder;');
const CovariantReader = etsVm.getClass('Ltest_generic_all/CovariantReader;');
const ContravariantWriter = etsVm.getClass('Ltest_generic_all/ContravariantWriter;');
const BoundedGenericClass = etsVm.getClass('Ltest_generic_all/BoundedGenericClass;');
const NestedGenericOuter = etsVm.getClass('Ltest_generic_all/NestedGenericOuter;');
const NestedGenericInner = etsVm.getClass('Ltest_generic_all/NestedGenericInner;');
const MultiTypeConstraint = etsVm.getClass('Ltest_generic_all/MultiTypeConstraint;');
const TypeDependency = etsVm.getClass('Ltest_generic_all/TypeDependency;');
const MultiDefault = etsVm.getClass('Ltest_generic_all/MultiDefault;');
const UnionConstraint = etsVm.getClass('Ltest_generic_all/UnionConstraint;');
const GenericMethodDefault = etsVm.getClass('Ltest_generic_all/GenericMethodDefault;');
const ConcreteRecursive = etsVm.getClass('Ltest_generic_all/ConcreteRecursive;');
const GenericFunctionType = etsVm.getClass('Ltest_generic_all/GenericFunctionType;');
const GenericWithFunctionParam = etsVm.getClass('Ltest_generic_all/GenericWithFunctionParam;');
const KeyOfTestClass = etsVm.getClass('Ltest_generic_all/KeyOfTestClass;');
const KeyOfGenericClass = etsVm.getClass('Ltest_generic_all/KeyOfGenericClass;');

const createAnimalContainer = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createAnimalContainer');
const createDogContainer = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createDogContainer');
const processWithSimpleConsumer = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'processWithSimpleConsumer');
const getFromSimpleProducer = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getFromSimpleProducer');
const testInvariance = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'testInvariance');
const readAnimalName = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'readAnimalName');
const readDogName = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'readDogName');
const writeToAnimal = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'writeToAnimal');
const writeToDog = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'writeToDog');
const createBoundedClass = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createBoundedClass');
const getBoundedItemName = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getBoundedItemName');
const getBoundedItemAge = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getBoundedItemAge');
const createNestedGeneric = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createNestedGeneric');
const getNestedValue = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getNestedValue');
const createMultiConstraint = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createMultiConstraint');
const getMultiConstraintAnimalName = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getMultiConstraintAnimalName');
const getMultiConstraintAge = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getMultiConstraintAge');
const createTypeDependency = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createTypeDependency');
const getTypeDependencyFirst = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getTypeDependencyFirst');
const getTypeDependencySecond = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getTypeDependencySecond');
const createMultiDefaultFull = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createMultiDefaultFull');
const createMultiDefaultPartial = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createMultiDefaultPartial');
const getMultiDefaultValues = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getMultiDefaultValues');
const createUnionConstraintAnimal = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createUnionConstraintAnimal');
const createUnionConstraintNClass = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createUnionConstraintNClass');
const getUnionConstraintAnimal = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getUnionConstraintAnimal');
const getUnionConstraintNClass = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getUnionConstraintNClass');
const createGenericMethodDefault = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createGenericMethodDefault');
const testGenericMethodDefault = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'testGenericMethodDefault');
const createConcreteRecursive = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createConcreteRecursive');
const getRecursiveData = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getRecursiveData');
const getRecursiveSelfValue = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getRecursiveSelfValue');
const createGenericFunctionType = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createGenericFunctionType');
const executeGenericFunction = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'executeGenericFunction');
const createGenericWithFunctionParam = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createGenericWithFunctionParam');
const testGenericFunctionParam = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'testGenericFunctionParam');
const createKeyOfGenericField1 = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createKeyOfGenericField1');
const createKeyOfGenericField2 = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createKeyOfGenericField2');
const getKeyOfValue = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getKeyOfValue');
const createGenericPair = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createGenericPair');
const getGenericPairKey = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getGenericPairKey');
const getGenericPairValue = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getGenericPairValue');
const createPartialObject = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createPartialObject');
const getPartialTitle = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getPartialTitle');
const createReadonlyObject = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createReadonlyObject');
const getReadonlyId = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getReadonlyId');
const getReadonlyName = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getReadonlyName');
const createRequiredObject = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createRequiredObject');
const getRequiredTitle = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getRequiredTitle');
const getRequiredDesc = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getRequiredDesc');
const createNonNullableValue = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'createNonNullableValue');
const getNonNullableLength = etsVm.getFunction('Ltest_generic_all/ETSGLOBAL;', 'getNonNullableLength');

const staVal = etsVm.getClass('Ltest_generic_all/ETSGLOBAL;').staVal;
const staObject = etsVm.getClass('Ltest_generic_all/ETSGLOBAL;').staObject;
const genericArrayInt = etsVm.getClass('Ltest_generic_all/ETSGLOBAL;').genericArrayInt;
const genericArrayString = etsVm.getClass('Ltest_generic_all/ETSGLOBAL;').genericArrayString;

class A {
  public data?: string;
  constructor(data: string) {
    this.data = data;
  }
}

class DynExtend extends ToExClass<DynExtend> {
  public dataT: string;
  constructor(dataT: string) {
    super(dataT);
    this.dataT = dataT;
  }
}

class DynExtend2<T> extends ToExClass<DynExtend2<T>> {
  public dataT: T;
  constructor(dataT: T) {
    super(dataT);
    this.dataT = dataT;
  }
}

class DynCircle extends GenericCircle<DynCircle> {
  public radius: number = 10;
}

class DynClassExG<T, V> extends GClass<V> {
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

class DynClassImG<V, T> {
  dataT?: T;
  dataV?: V;
  constructor(arg1: V, arg2: T) {
    this.dataT = arg2;
    this.dataV = arg1;
  }
  setDataNew(arg1: V, arg2: T): void {
    this.dataV = arg1;
    this.dataT = arg2;
  }
  getDataT(): T | undefined {
    return this.dataT;
  }
  getDataV(): V | undefined {
    return this.dataV;
  }
}

class Animal { name: string = 'Animal'; }
class Dog extends Animal { name: string = 'dog'; }

function testGenericFunction(): void {
  let resString = fooIdentity<string>('hello');
  ASSERT_TRUE(resString === 'hello');

  let resNumber = fooIdentity<number>(42);
  ASSERT_TRUE(resNumber === 42);

  let resNClass = fooIdentity<NClass>(new NClass('test'));
  ASSERT_TRUE(resNClass.data === 'test');

  let multiParam = fooMultiParam<string, number>('first', 100);
  ASSERT_TRUE(multiParam === 'first');

  let unionRes = fooUnionReturn<string, A>('strVal', new A('classA'));
  ASSERT_TRUE(unionRes === 'strVal');
}

function testGenericClass(): void {
  let gClassString = new GClass<string>('string');
  ASSERT_TRUE(gClassString.getData() === 'string');

  let gClassNClass = new GClass<NClass>(new NClass('NClass'));
  ASSERT_TRUE(gClassNClass.getData().data === 'NClass');

  let gClassNested = new GClass<GClass<string>>(new GClass<string>('nested'));
  ASSERT_TRUE(gClassNested.getData().getData() === 'nested');

  ASSERT_TRUE(GClass.staticData === 'STATICDATA');

  let result = gClassString.transfer<string>((val: string) => val.toUpperCase());
  ASSERT_TRUE(result === 'STRING');
}

function testGenericClassExtends(): void {
  let exNClassString = new GExNClass<string>('string');
  ASSERT_TRUE(exNClassString.getData() === 'string');
  ASSERT_TRUE(exNClassString.data === 'NClassstring');

  let dgExNClassString = new DGExNClass<string>('string');
  ASSERT_TRUE(dgExNClassString.getData() === 'string');
  ASSERT_TRUE(dgExNClassString.data === 'NClassstring');

  let exGClassBasic = new GExGClass<string, string>('thisString', 'superString');
  ASSERT_TRUE(exGClassBasic.getDataT() === 'thisString');
  ASSERT_TRUE(exGClassBasic.getData() === 'superString');

  let dgExGClassBasic = new DGExGClass<string, string>('thisString', 'superString');
  ASSERT_TRUE(dgExGClassBasic.getDataT() === 'thisString');
  ASSERT_TRUE(dgExGClassBasic.getData() === 'superString');
}

function testGenericClassImplements(): void {
  let gImClass = new GImClass<string, string>();
  ASSERT_TRUE(gImClass !== undefined);

  let gImNClass = new GImNClass<string, string>();
  ASSERT_TRUE(gImNClass !== undefined);

  let gImDClass = new GImDClass<string, string>();
  gImDClass.dataT = 'testValue';
  ASSERT_TRUE(gImDClass.dataT === 'testValue');
}

function testTsClassExtendsEtsGenericClass(): void {
  let dynClassExG = new DynClassExG<string, string>('thisString', 'superString');
  ASSERT_TRUE(dynClassExG.getDataT() === 'thisString');
  ASSERT_TRUE(dynClassExG.getData() === 'superString');

  let dynClassExGComplex = new DynClassExG<A, GClass<NClass>>(
    new A('classA'),
    new GClass<NClass>(new NClass('NClass'))
  );
  ASSERT_TRUE(dynClassExGComplex.getDataT().data === 'classA');
  ASSERT_TRUE(dynClassExGComplex.getData().getData().data === 'NClass');
}

function testTsClassWithGenericFields(): void {
  let dynClassImG = new DynClassImG<string, string>('VString', 'TString');
  ASSERT_TRUE(dynClassImG.getDataT() === 'TString');
  ASSERT_TRUE(dynClassImG.getDataV() === 'VString');
}

function testGenericDefaultTypeParam(): void {
  let fDefaultString = fooDefault('hi');
  ASSERT_TRUE(fDefaultString === 'hi');

  let fDefaultNumber = fooDefault<number>(100);
  ASSERT_TRUE(fDefaultNumber === 100);

  let gClassDefaultString = new GClassDefault('defaultString');
  ASSERT_TRUE(gClassDefaultString.data === 'defaultString');

  let gClassDefaultNumber = new GClassDefault<number>(42);
  ASSERT_TRUE(gClassDefaultNumber.data === 42);
}

function testGenericExtendsConstraint(): void {
  let dynExtend = new DynExtend('stringthis');
  ASSERT_TRUE(dynExtend.dataT === 'stringthis');

  let gExtendClass = new GClassExtend<DynExtend>();
  gExtendClass.data = dynExtend;
  ASSERT_TRUE(gExtendClass.data?.dataT === 'stringthis');

  let gExtendFoo = fooExtends<DynExtend>(dynExtend);
  ASSERT_TRUE(gExtendFoo.dataT === 'stringthis');

  let dynExtend2 = new DynExtend2<string>('stringthis2');
  ASSERT_TRUE(dynExtend2.dataT === 'stringthis2');

  let dynExtend2Num = new DynExtend2<number>(123);
  ASSERT_TRUE(dynExtend2Num.dataT === 123);
}

function testGenericRecursiveType(): void {
  let dynCircle = new DynCircle();
  ASSERT_TRUE(dynCircle.radius === 10);

  dynCircle.data = dynCircle;
  ASSERT_TRUE(dynCircle.data?.radius === 10);
}

function testGenericNullable(): void {
  let res1 = fooNullable<string>('value');
  ASSERT_TRUE(res1 === 'value');

  let res2 = fooNullable<string>(undefined);
  ASSERT_TRUE(res2 === undefined);

  let res3 = fooNullable<NClass>(new NClass('test'));
  ASSERT_TRUE(res3?.data === 'test');

  let gNullableStr = new GNullableClass<string>('initial');
  ASSERT_TRUE(gNullableStr.hasData() === true);
  ASSERT_TRUE(gNullableStr.getData() === 'initial');

  gNullableStr.setData('updated');
  ASSERT_TRUE(gNullableStr.getData() === 'updated');
}

function testGenericPairClass(): void {
  let pair1 = new GPairClass<string, number>('text', 42);
  ASSERT_TRUE(pair1.getFirst() === 'text');
  ASSERT_TRUE(pair1.getSecond() === 42);

  pair1.setFirst('updated');
  pair1.setSecond(100);
  ASSERT_TRUE(pair1.getFirst() === 'updated');
  ASSERT_TRUE(pair1.getSecond() === 100);

  let pair2 = new GPairClass<A, NClass>(new A('classA'), new NClass('NClass'));
  ASSERT_TRUE(pair2.getFirst().data === 'classA');
  ASSERT_TRUE(pair2.getSecond().data === 'NClass');
}

function testGenericMethod(): void {
  let gMethodStr = new GMethodClass<string>('data');
  let mapped = gMethodStr.process<string>((val: string) => val.toUpperCase());
  ASSERT_TRUE(mapped === 'DATA');

  let lengthResult = gMethodStr.process<number>((val: string) => val.length);
  ASSERT_TRUE(lengthResult === 4);

  let gMethodImplStr = new GMethodImplClass<string>('initial');
  ASSERT_TRUE(gMethodImplStr.getValue() === 'initial');
  gMethodImplStr.setValue('updated');
  ASSERT_TRUE(gMethodImplStr.getValue() === 'updated');
}

function testAnimalHierarchy(): void {
  let animalContainer = createAnimalContainer();
  ASSERT_TRUE(animalContainer.getItem().name === 'Animal');

  let dogContainer = createDogContainer();
  ASSERT_TRUE(dogContainer.getItem().name === 'Dog');

  let gClassAnimal = new GClass<EtsAnimal>(new EtsAnimal());
  ASSERT_TRUE(gClassAnimal.getData().name === 'Animal');

  let gClassDog = new GClass<EtsDog>(new EtsDog());
  ASSERT_TRUE(gClassDog.getData().name === 'Dog');

  let gClassCat = new GClass<EtsCat>(new EtsCat());
  ASSERT_TRUE(gClassCat.getData().name === 'Cat');
}

function testConsumerProducer(): void {
  let animalConsumer = new SimpleConsumer<EtsAnimal>();
  processWithSimpleConsumer<EtsAnimal>(animalConsumer, new EtsAnimal());
  ASSERT_TRUE(animalConsumer.hasConsumed() === true);
  ASSERT_TRUE(animalConsumer.getConsumed()?.name === 'Animal');

  let animalProducer = new SimpleProducer<EtsAnimal>(new EtsAnimal());
  let animal = getFromSimpleProducer<EtsAnimal>(animalProducer);
  ASSERT_TRUE(animal.name === 'Animal');
}

function testInvariantHolder(): void {
  let holderStr = new InvariantHolder<string>('test');
  ASSERT_TRUE(holderStr.getData() === 'test');

  holderStr.setData('updated');
  ASSERT_TRUE(holderStr.getData() === 'updated');

  let result = testInvariance(holderStr);
  ASSERT_TRUE(result === 'updated');

  let holder1 = new InvariantHolder<string>('first');
  let holder2 = new InvariantHolder<string>('second');
  holder1.exchange(holder2);
  ASSERT_TRUE(holder1.getData() === 'second');
  ASSERT_TRUE(holder2.getData() === 'first');
}

function testCovariantContravariant(): void {
  let animalReader = new CovariantReader<EtsAnimal>(new EtsAnimal());
  ASSERT_TRUE(animalReader.readName() === 'Animal');
  ASSERT_TRUE(readAnimalName(animalReader) === 'Animal');

  let dogReader = new CovariantReader<EtsDog>(new EtsDog());
  ASSERT_TRUE(dogReader.readName() === 'Dog');
  ASSERT_TRUE(readDogName(dogReader) === 'Dog');

  let animalWriter = new ContravariantWriter<EtsAnimal>();
  writeToAnimal(animalWriter);
  ASSERT_TRUE(animalWriter.getWrittenName() === 'Animal');

  let dogWriter = new ContravariantWriter<EtsDog>();
  writeToDog(dogWriter);
  ASSERT_TRUE(dogWriter.getWrittenName() === 'Dog');
}

function testBoundedGeneric(): void {
  let boundedClass = createBoundedClass();
  ASSERT_TRUE(boundedClass.getItemName() === 'Animal');
  ASSERT_TRUE(boundedClass.getItemAge() === 5);

  ASSERT_TRUE(getBoundedItemName(boundedClass) === 'Animal');
  ASSERT_TRUE(getBoundedItemAge(boundedClass) === 5);

  let directBounded = new BoundedGenericClass<AnimalWithAge>(new AnimalWithAge());
  ASSERT_TRUE(directBounded.getItemName() === 'Animal');
  ASSERT_TRUE(directBounded.getItemAge() === 5);
}

function testNestedGeneric(): void {
  let nestedOuter = createNestedGeneric();
  ASSERT_TRUE(nestedOuter.getValue() === 'nested');

  let inner = nestedOuter.getInner();
  ASSERT_TRUE(inner.getValue() === 'nested');

  ASSERT_TRUE(getNestedValue(nestedOuter) === 'nested');
}

function testMultiTypeConstraint(): void {
  let multiConstraint = createMultiConstraint();
  ASSERT_TRUE(multiConstraint.getAnimalName() === 'Dog');
  ASSERT_TRUE(multiConstraint.getAgedAge() === 3);

  ASSERT_TRUE(getMultiConstraintAnimalName(multiConstraint) === 'Dog');
  ASSERT_TRUE(getMultiConstraintAge(multiConstraint) === 3);
}

function testTypeParameterDependency(): void {
  let holder = createTypeDependency();
  ASSERT_TRUE(getTypeDependencyFirst(holder) === 'Animal');
  ASSERT_TRUE(getTypeDependencySecond(holder) === 'Dog');

  let directHolder = new TypeDependency<EtsAnimal, EtsDog>(new EtsAnimal(), new EtsDog());
  ASSERT_TRUE(directHolder.getFirst().name === 'Animal');
  ASSERT_TRUE(directHolder.getSecond().name === 'Dog');
  ASSERT_TRUE(directHolder.getSecondAsFirst().name === 'Dog');
}

function testMultipleDefaultTypeParams(): void {
  let full = createMultiDefaultFull();
  ASSERT_TRUE(full.getValue1() === true);
  ASSERT_TRUE(full.getValue2() === 42);
  ASSERT_TRUE(full.getValue3() === 'test');

  let partial = createMultiDefaultPartial();
  ASSERT_TRUE(partial.getValue1() === true);
  ASSERT_TRUE(partial.getValue2() === 100);
  ASSERT_TRUE(partial.getValue3() === 'partial');

  let directFull = new MultiDefault<boolean, number, string>(true, 200, 'direct');
  ASSERT_TRUE(directFull.getValue1() === true);
  ASSERT_TRUE(directFull.getValue2() === 200);
  ASSERT_TRUE(directFull.getValue3() === 'direct');
}

function testUnionTypeConstraint(): void {
  let animalHolder = createUnionConstraintAnimal();
  ASSERT_TRUE(getUnionConstraintAnimal(animalHolder).name === 'Animal');

  let nClassHolder = createUnionConstraintNClass();
  ASSERT_TRUE(getUnionConstraintNClass(nClassHolder).data === 'unionNClass');

  let directAnimal = new UnionConstraint<EtsAnimal>(new EtsDog());
  ASSERT_TRUE(directAnimal.getItem().name === 'Dog');
}

function testGenericMethodDefaultClass(): void {
  let holder = createGenericMethodDefault();
  ASSERT_TRUE(testGenericMethodDefault(holder) === 'defaultMethod');
  ASSERT_TRUE(holder.getData() === 'defaultMethod');

  let directHolder = new GenericMethodDefault<string>('hello');
  ASSERT_TRUE(directHolder.processWithClassDefault() === 'hello');
}

function testRecursiveGenericType(): void {
  let cr = createConcreteRecursive();
  ASSERT_TRUE(getRecursiveData(cr) === 'recursive');
  ASSERT_TRUE(getRecursiveSelfValue(cr) === 200);

  let direct = new ConcreteRecursive('direct', 300);
  direct.setSelf(direct);
  ASSERT_TRUE(direct.getData() === 'direct');
  ASSERT_TRUE(direct.getSelf()?.value === 300);
}

function testGenericFunctionType(): void {
  let holder = createGenericFunctionType();
  ASSERT_TRUE(executeGenericFunction(holder, 'hello') === 'HELLO');

  let directHolder = new GenericFunctionType<string>((s: string) => s + '_processed');
  ASSERT_TRUE(directHolder.execute('test') === 'test_processed');
}

function testGenericWithFunctionParam(): void {
  let holder = createGenericWithFunctionParam();
  ASSERT_TRUE(testGenericFunctionParam(holder) === 5);

  let directHolder = new GenericWithFunctionParam<string, number>('world', (s: string) => s.length * 2);
  ASSERT_TRUE(directHolder.transform() === 10);
}

function testKeyOfConstraint(): void {
  let field1Key = createKeyOfGenericField1();
  ASSERT_TRUE(field1Key.getKey() === 'field1');

  let field2Key = createKeyOfGenericField2();
  ASSERT_TRUE(field2Key.getKey() === 'field2');

  let directField1 = new KeyOfGenericClass<'field1'>('field1');
  ASSERT_TRUE(directField1.getKey() === 'field1');

  let directUnion = new KeyOfGenericClass<'field1' | 'field2'>('field1');
  ASSERT_TRUE(getKeyOfValue(directUnion) === 'field1');
}

function testTypeAliasGeneric(): void {
  ASSERT_TRUE(genericArrayInt.length === 3);
  ASSERT_TRUE(genericArrayInt[0] === 1);
  ASSERT_TRUE(genericArrayInt[1] === 2);
  ASSERT_TRUE(genericArrayInt[2] === 3);

  ASSERT_TRUE(genericArrayString.length === 3);
  ASSERT_TRUE(genericArrayString[0] === 'a');
  ASSERT_TRUE(genericArrayString[1] === 'b');
  ASSERT_TRUE(genericArrayString[2] === 'c');

  let pair = createGenericPair();
  ASSERT_TRUE(getGenericPairKey(pair) === 'age');
  ASSERT_TRUE(getGenericPairValue(pair) === 30);
}

function testUtilityTypes(): void {
  let partialObj = createPartialObject();
  ASSERT_TRUE(getPartialTitle(partialObj) === 'testTitle');

  let readonlyObj = createReadonlyObject();
  ASSERT_TRUE(getReadonlyId(readonlyObj) === 1);
  ASSERT_TRUE(getReadonlyName(readonlyObj) === 'readonlyName');

  let requiredObj = createRequiredObject();
  ASSERT_TRUE(getRequiredTitle(requiredObj) === 'reqTitle');
  ASSERT_TRUE(getRequiredDesc(requiredObj) === 'reqDesc');

  let nonNullVal = createNonNullableValue();
  ASSERT_TRUE(getNonNullableLength(nonNullVal) === 12);
}

function testEtsExportedVariables(): void {
  ASSERT_TRUE(staVal === 1);
  ASSERT_TRUE(staObject.data === 'dyn');
}

function main(): void {
  testGenericFunction();
  testGenericClass();
  testGenericClassExtends();
  testGenericClassImplements();
  testTsClassExtendsEtsGenericClass();
  testTsClassWithGenericFields();
  testGenericDefaultTypeParam();
  testGenericExtendsConstraint();
  testGenericRecursiveType();
  testGenericNullable();
  testGenericPairClass();
  testGenericMethod();
  testAnimalHierarchy();
  testConsumerProducer();
  testInvariantHolder();
  testCovariantContravariant();
  testBoundedGeneric();
  testNestedGeneric();
  testMultiTypeConstraint();
  testTypeParameterDependency();
  testMultipleDefaultTypeParams();
  testUnionTypeConstraint();
  testGenericMethodDefaultClass();
  testRecursiveGenericType();
  testGenericFunctionType();
  testGenericWithFunctionParam();
  testKeyOfConstraint();
  testTypeAliasGeneric();
  testUtilityTypes();
  testEtsExportedVariables();
}

main();