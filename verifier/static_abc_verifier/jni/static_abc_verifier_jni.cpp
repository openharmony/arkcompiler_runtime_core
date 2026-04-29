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

#include <jni.h>
#include "verify_api.h"

#include <cstring>
#include <string>

static constexpr int ERROR_BUF_SIZE = 4096;

extern "C" {
/*
 * Class:     com_oh_ark_verifier_StaticAbcVerifier
 * Method:    nativeVerifyFile
 * Signature: (Ljava/lang/String;)Lcom/oh/ark/verifier/VerifyResult;
 */
JNIEXPORT jobject JNICALL Java_com_oh_ark_verifier_StaticAbcVerifier_nativeVerifyFile(
    JNIEnv *env, jclass /* cls */, jstring jFilePath)
{
    jclass resultClass = env->FindClass("com/oh/ark/verifier/VerifyResult");
    jmethodID resultCtor = env->GetMethodID(resultClass, "<init>", "(ZILjava/lang/String;)V");

    if (jFilePath == nullptr) {
        jstring errMsg = env->NewStringUTF("File path is null");
        return env->NewObject(resultClass, resultCtor, JNI_FALSE, 1, errMsg);
    }

    const char *filePath = env->GetStringUTFChars(jFilePath, nullptr);
    if (filePath == nullptr) {
        jstring errMsg = env->NewStringUTF("Failed to get file path string");
        return env->NewObject(resultClass, resultCtor, JNI_FALSE, 1, errMsg);
    }

    char errorBuf[ERROR_BUF_SIZE] = {0};
    int ret = StaticAbcVerifyFile(filePath, errorBuf, ERROR_BUF_SIZE);

    env->ReleaseStringUTFChars(jFilePath, filePath);

    jboolean valid = (ret == 0) ? JNI_TRUE : JNI_FALSE;
    jstring errMsg = env->NewStringUTF(errorBuf);
    return env->NewObject(resultClass, resultCtor, valid, static_cast<jint>(ret), errMsg);
}

}  // extern "C"
