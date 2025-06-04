/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
import { getArray, returnCustomArray, createStringRecord,createInt8Array, RecordObject,handler} from "indexed_type"
import {createProxy} from "proxy"

const arr = ['A','B','C'];

function getArrValueByIndex():boolean{
    const jsArray = getArray(arr) as string[];
    return jsArray[0] == 'A';
}

function changeArrValueByIndex():boolean{
    let jsArray = getArray(arr) as string[];
    jsArray[0] = 'D';
    return jsArray[0] == 'D';
}

function checkLengthArr():boolean{
    const jsArray:string[] = getArray(arr) as string[];
    return jsArray.length as number == arr.length as number
}
function checkAllArrValue():boolean{
    const jsArray:string[] = getArray(arr) as string[];
    return jsArray[0] == 'A' && jsArray[1] == 'B' && jsArray[2] == 'C'
}


function getCustomArrValueByIndex():boolean{
    const jsArray = returnCustomArray(arr);
    return jsArray[0] == 'A';
}

function changeCustomArrValueByIndex():boolean{
    let jsArray = returnCustomArray(arr) as string[];
    jsArray[0] = 'D';
    return jsArray[0] == 'D';
}
function checkLengthCustomArr():boolean{
    const jsArray:string[] = returnCustomArray(arr);
    return jsArray.length as number == arr.length as number;
}

function checkAllCustomArrValue():boolean{
    const jsArray:string[] = returnCustomArray(arr);
    return jsArray[0] == 'A' && jsArray[1] == 'B' && jsArray[2] == 'C';
}

function getRecordValue():boolean{
   const recordObj = createStringRecord();
   return recordObj.key1 == 'A';
}

function changeRecordValue():boolean{
    let recordObj = createStringRecord();
    recordObj.key1 = 'D';
    return recordObj.key1 == 'D';
 }
// NOTE(andreypetukhov) enable after fixibng #17821
//  function getRecordValueIndex():boolean{
//     const recordObj:Record<string, string> = createStringRecord();
//     return recordObj['key1'] == 'A'
//  }
// NOTE(andreypetukhov) enable after fixibng #17821
//  function changeRecordValueIndex():boolean{
//     let recordObj:Record<string, string> = createStringRecord();
//     recordObj['key1'] = 'F';
//     return recordObj['key1'] == 'F';
//  }
// NOTE(andreypetukhov) enable after fixibng #17821
// function checkRecordValueLength():boolean{
//     const recordObj:Record<string, string> = createStringRecord();
//     return Object.entries(recordObj).length as number == 3
//  }

// NOTE(andreypetukhov) enable after fixibng #17821
//  function checkRecordValue():boolean{
//   const recordObj:Record<string, string> = createStringRecord();
//   return recordObj['key1'] === 'A' && recordObj['key2'] === 'B' && recordObj['key3'] === 'C'
// }

function getTypedArrayValueByIndex():boolean{
    const jsTypedArray = createInt8Array(3);
    return jsTypedArray[0] as number == 0;
}
// NOTE(andreypetukhov) enable after fixibng #18131
// function changeTypedArrayValueByIndex():boolean{
//     const jsTypedArray = createInt8Array(3) as Int8Array;
//     jsTypedArray[1] = 99;
//     return jsTypedArray[1] as number == 99;
// }

function checkLengthTypedArray():boolean{
    const jsTypedArray = createInt8Array(3) as Int8Array;
    return jsTypedArray.length as number == 3 as number
}

function checkAllTypedArrayValue():boolean{
    const jsTypedArray = createInt8Array(3) as Int8Array;
    return jsTypedArray[0] as number == 0 && jsTypedArray[1] as number == 1 && jsTypedArray[2] as number == 2
 }

 const testObj:RecordObject = {
    'keyA':'A',
    'keyB':'B'
 }

 function getValueFromProxyRecord():boolean{
    const proxyRecord = createProxy(testObj,handler)
    console.log(proxyRecord.keyA)
    return proxyRecord.keyA == 'A'
 }

 function changeValueFromProxyRecord():boolean{
    const proxyRecord = createProxy(testObj,handler)
    proxyRecord.keyB = 'T'
    return proxyRecord.keyB == 'T'
 }
// NOTE(andreypetukhov) enable after fixibng #18238
//  function getValueFromProxyByKeyRecord():boolean{
//     const proxyRecord = createProxy(testObj,handler)
//     return proxyRecord['keyA'] as string == 'A'
//  }
