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

const fooIdentity = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'fooIdentity');
const fooMultiParam = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'fooMultiParam');
const fooUnionReturn = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'fooUnionReturn');
const fooDefault = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'fooDefault');
const fooNullable = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'fooNullable');
const fooExtends = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'fooExtends');

const NClass = etsVm.getClass('Lgeneric_all/NClass;');
const Base = etsVm.getClass('Lgeneric_all/Base;');
const Child = etsVm.getClass('Lgeneric_all/Child;');
const GClass = etsVm.getClass('Lgeneric_all/GClass;');
const GExNClass = etsVm.getClass('Lgeneric_all/GExNClass;');
const GExGClass = etsVm.getClass('Lgeneric_all/GExGClass;');
const GImClass = etsVm.getClass('Lgeneric_all/GImClass;');
const GImNClass = etsVm.getClass('Lgeneric_all/GImNClass;');
const GClassDefault = etsVm.getClass('Lgeneric_all/GClassDefault;');
const ToExClass = etsVm.getClass('Lgeneric_all/ToExClass;');
const GClassExtend = etsVm.getClass('Lgeneric_all/GClassExtend;');
const GenericCircle = etsVm.getClass('Lgeneric_all/GenericCircle;');
const DynExtend = etsVm.getClass('Lgeneric_all/DynExtend;');
const DynCircle = etsVm.getClass('Lgeneric_all/DynCircle;');
const GNullableClass = etsVm.getClass('Lgeneric_all/GNullableClass;');
const GPairClass = etsVm.getClass('Lgeneric_all/GPairClass;');
const GMethodClass = etsVm.getClass('Lgeneric_all/GMethodClass;');
const GMethodImplClass = etsVm.getClass('Lgeneric_all/GMethodImplClass;');

const Animal = etsVm.getClass('Lgeneric_all/Animal;');
const Dog = etsVm.getClass('Lgeneric_all/Dog;');
const Cat = etsVm.getClass('Lgeneric_all/Cat;');
const AnimalWithAge = etsVm.getClass('Lgeneric_all/AnimalWithAge;');
const AgedDog = etsVm.getClass('Lgeneric_all/AgedDog;');
const GenericContainer = etsVm.getClass('Lgeneric_all/GenericContainer;');
const SimpleConsumer = etsVm.getClass('Lgeneric_all/SimpleConsumer;');
const SimpleProducer = etsVm.getClass('Lgeneric_all/SimpleProducer;');
const InvariantHolder = etsVm.getClass('Lgeneric_all/InvariantHolder;');
const CovariantReader = etsVm.getClass('Lgeneric_all/CovariantReader;');
const ContravariantWriter = etsVm.getClass('Lgeneric_all/ContravariantWriter;');
const BoundedGenericClass = etsVm.getClass('Lgeneric_all/BoundedGenericClass;');
const NestedGenericOuter = etsVm.getClass('Lgeneric_all/NestedGenericOuter;');
const NestedGenericInner = etsVm.getClass('Lgeneric_all/NestedGenericInner;');
const MultiTypeConstraint = etsVm.getClass('Lgeneric_all/MultiTypeConstraint;');
const TypeDependency = etsVm.getClass('Lgeneric_all/TypeDependency;');
const MultiDefault = etsVm.getClass('Lgeneric_all/MultiDefault;');
const UnionConstraint = etsVm.getClass('Lgeneric_all/UnionConstraint;');
const GenericMethodDefault = etsVm.getClass('Lgeneric_all/GenericMethodDefault;');
const ConcreteRecursive = etsVm.getClass('Lgeneric_all/ConcreteRecursive;');
const GenericFunctionType = etsVm.getClass('Lgeneric_all/GenericFunctionType;');
const GenericWithFunctionParam = etsVm.getClass('Lgeneric_all/GenericWithFunctionParam;');
const KeyOfTestClass = etsVm.getClass('Lgeneric_all/KeyOfTestClass;');
const KeyOfGenericClass = etsVm.getClass('Lgeneric_all/KeyOfGenericClass;');

const createAnimalContainer = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createAnimalContainer');
const createDogContainer = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createDogContainer');
const processWithSimpleConsumer = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'processWithSimpleConsumer');
const getFromSimpleProducer = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getFromSimpleProducer');
const testInvariance = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'testInvariance');
const readAnimalName = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'readAnimalName');
const readDogName = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'readDogName');
const writeToAnimal = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'writeToAnimal');
const writeToDog = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'writeToDog');
const createBoundedClass = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createBoundedClass');
const getBoundedItemName = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getBoundedItemName');
const getBoundedItemAge = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getBoundedItemAge');
const createNestedGeneric = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createNestedGeneric');
const getNestedValue = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getNestedValue');
const createMultiConstraint = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createMultiConstraint');
const getMultiConstraintAnimalName = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getMultiConstraintAnimalName');
const getMultiConstraintAge = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getMultiConstraintAge');
const createTypeDependency = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createTypeDependency');
const getTypeDependencyFirst = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getTypeDependencyFirst');
const getTypeDependencySecond = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getTypeDependencySecond');
const createMultiDefaultFull = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createMultiDefaultFull');
const createMultiDefaultPartial = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createMultiDefaultPartial');
const getMultiDefaultValues = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getMultiDefaultValues');
const createUnionConstraintAnimal = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createUnionConstraintAnimal');
const createUnionConstraintNClass = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createUnionConstraintNClass');
const getUnionConstraintAnimal = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getUnionConstraintAnimal');
const getUnionConstraintNClass = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getUnionConstraintNClass');
const createGenericMethodDefault = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createGenericMethodDefault');
const testGenericMethodDefault = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'testGenericMethodDefault');
const testGenericMethodExplicit = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'testGenericMethodExplicit');
const createConcreteRecursive = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createConcreteRecursive');
const getRecursiveData = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getRecursiveData');
const getRecursiveSelfValue = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getRecursiveSelfValue');
const createGenericFunctionType = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createGenericFunctionType');
const executeGenericFunction = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'executeGenericFunction');
const createGenericWithFunctionParam = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createGenericWithFunctionParam');
const testGenericFunctionParam = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'testGenericFunctionParam');
const createKeyOfGenericField1 = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createKeyOfGenericField1');
const createKeyOfGenericField2 = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createKeyOfGenericField2');
const getKeyOfValue = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getKeyOfValue');
const createGenericPair = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createGenericPair');
const getGenericPairKey = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getGenericPairKey');
const getGenericPairValue = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getGenericPairValue');
const createPartialObject = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createPartialObject');
const getPartialTitle = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getPartialTitle');
const createReadonlyObject = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createReadonlyObject');
const getReadonlyId = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getReadonlyId');
const getReadonlyName = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getReadonlyName');
const createRequiredObject = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createRequiredObject');
const getRequiredTitle = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getRequiredTitle');
const getRequiredDesc = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getRequiredDesc');
const createNonNullableValue = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'createNonNullableValue');
const getNonNullableLength = etsVm.getFunction('Lgeneric_all/ETSGLOBAL;', 'getNonNullableLength');

const dynVal = etsVm.getClass('Lgeneric_all/ETSGLOBAL;').dynVal;
const dynObject = etsVm.getClass('Lgeneric_all/ETSGLOBAL;').dynObject;
const nClassString = etsVm.getClass('Lgeneric_all/ETSGLOBAL;').nClassString;
const genericArrayInt = etsVm.getClass('Lgeneric_all/ETSGLOBAL;').genericArrayInt;
const genericArrayString = etsVm.getClass('Lgeneric_all/ETSGLOBAL;').genericArrayString;

class A {
  data?: string;
  constructor(data: string) {
    this.data = data;
  }
}

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

  let exGClassBasic = new GExGClass<string, string>('thisString', 'superString');
  ASSERT_TRUE(exGClassBasic.getDataT() === 'thisString');
  ASSERT_TRUE(exGClassBasic.getData() === 'superString');
}

function testGenericClassImplements(): void {
  let gImClass = new GImClass<string, string>();
  ASSERT_TRUE(gImClass !== undefined);

  let gImNClass = new GImNClass<string, string>();
  ASSERT_TRUE(gImNClass !== undefined);
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

  let gClassAnimal = new GClass<Animal>(new Animal());
  ASSERT_TRUE(gClassAnimal.getData().name === 'Animal');

  let gClassDog = new GClass<Dog>(new Dog());
  ASSERT_TRUE(gClassDog.getData().name === 'Dog');

  let gClassCat = new GClass<Cat>(new Cat());
  ASSERT_TRUE(gClassCat.getData().name === 'Cat');
}

function testConsumerProducer(): void {
  let animalConsumer = new SimpleConsumer<Animal>();
  processWithSimpleConsumer<Animal>(animalConsumer, new Animal());
  ASSERT_TRUE(animalConsumer.hasConsumed() === true);
  ASSERT_TRUE(animalConsumer.getConsumed()?.name === 'Animal');

  let animalProducer = new SimpleProducer<Animal>(new Animal());
  let animal = getFromSimpleProducer<Animal>(animalProducer);
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
  let animalReader = new CovariantReader<Animal>(new Animal());
  ASSERT_TRUE(animalReader.readName() === 'Animal');
  ASSERT_TRUE(readAnimalName(animalReader) === 'Animal');

  let dogReader = new CovariantReader<Dog>(new Dog());
  ASSERT_TRUE(dogReader.readName() === 'Dog');
  ASSERT_TRUE(readDogName(dogReader) === 'Dog');

  let animalWriter = new ContravariantWriter<Animal>();
  writeToAnimal(animalWriter);
  ASSERT_TRUE(animalWriter.getWrittenName() === 'Animal');

  let dogWriter = new ContravariantWriter<Dog>();
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

  let directHolder = new TypeDependency<Animal, Dog>(new Animal(), new Dog());
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

  let directAnimal = new UnionConstraint<Animal>(new Dog());
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
  ASSERT_TRUE(dynVal === 1);
  ASSERT_TRUE(dynObject.data === 'dyn');
  ASSERT_TRUE(nClassString.data === 'string');
}

function main(): void {
  testGenericFunction();
  testGenericClass();
  testGenericClassExtends();
  testGenericClassImplements();
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