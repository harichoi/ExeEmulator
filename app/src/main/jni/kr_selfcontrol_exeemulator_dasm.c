/*
 * com_example_exeemulator_dasm.cpp
 *
 *  Created on: 2016. 1. 4.
 *      Author: kes
 */



/*
#include "com_example_exeemulator_dasm.h"
#include "dasm.h"

JNIEXPORT jint JNICALL Java_com_example_exeemulator_dasm_fileLoad(JNIEnv *pEnv, jstring jstr)
{
	const char *str = pEnv->GetStringUTFChars(jstr,0);
	fileLoad(const_cast<char*>(str),1000,1000);

    return 0;
}

JNIEXPORT jstring JNICALL Java_com_example_exeemulator_dasm_printBuffer(JNIEnv *pEnv, jobject obj)
{
	const char *str = getPrintBuffer();

    return pEnv->NewStringUTF(str);
}
*/

#include <string.h>
#include <android/log.h>
#include "kr_selfcontrol_exeemulator_dasm.h"
#include "dasm.h"

#define DEBUG_TAG "C_libG"

bool IsUTF8(const void *pBuffer, long size) {
    bool IsUTF8 = 1;
    unsigned char *start = (unsigned char *) pBuffer;
    unsigned char *end = (unsigned char *) pBuffer + size;
    while (start < end) {
        if (*start < 0x80) // (10000000): value less then 0x80 ASCII char
        {
            start++;
        } else if (*start < (0xC0)) // (11000000): between 0x80 and 0xC0 UTF-8 char
        {
            IsUTF8 = 0;
            break;
        }
        else if (*start < (0xE0)) // (11100000): 2 bytes UTF-8 char
        {
            if (start >= end - 1) break;
            if ((start[1] & (0xC0)) != 0x80) {
                IsUTF8 = 0;
                break;
            }
            start += 2;
        } else if (*start < (0xF0)) // (11110000): 3 bytes UTF-8 char
        {
            if (start >= end - 2) break;
            if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80) {
                IsUTF8 = 0;
                break;
            }
            start += 3;
        } else {
            IsUTF8 = 0;
            break;
        }
    }
    return IsUTF8;
}

JNIEXPORT jstring JNICALL Java_kr_selfcontrol_exeemulator_dasm_readNext(JNIEnv *pEnv, jobject obj) {
    char *str = readNext();

    if (IsUTF8(str, strlen(str)))
        return (*pEnv)->NewStringUTF(pEnv, str);
    return (*pEnv)->NewStringUTF(pEnv, "???");
}

JNIEXPORT jstring JNICALL Java_kr_selfcontrol_exeemulator_dasm_hahaha(JNIEnv *pEnv, jobject obj) {

    return (*pEnv)->NewStringUTF(pEnv, "test");
}

JNIEXPORT jint JNICALL Java_kr_selfcontrol_exeemulator_dasm_fileLoad(JNIEnv *pEnv, jobject obj,
                                                                     jstring jstr) {
    char *str = (*pEnv)->GetStringUTFChars(pEnv, jstr, 0);
    fileLoad(str, 10000, 10000);
    return 0;
}

JNIEXPORT jstring JNICALL Java_kr_selfcontrol_exeemulator_dasm_getPrintBuffer(JNIEnv *pEnv,
                                                                              jobject obj) {
    char *str = getPrintBuffer();
    if (str) {
        if (IsUTF8(str, strlen(str)))
            return (*pEnv)->NewStringUTF(pEnv, str);
        return (*pEnv)->NewStringUTF(pEnv, "???");
    }
    return NULL;
}


JNIEXPORT void JNICALL Java_kr_selfcontrol_exeemulator_dasm_callBackTest (JNIEnv *pEnv, jobject obj){
//    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Call JNI_callBackTest");
    JavaVM *jvm;
    (*pEnv)->GetJavaVM(pEnv,&jvm);
    (*jvm)->AttachCurrentThreadAsDaemon(jvm,&pEnv,NULL);
    jclass cls =(*pEnv)->GetObjectClass(pEnv,obj);
    //jclass cls = (*pEnv)->FindClass(pEnv,"kr/selfcontrol/exeemulator/dasm");
    if(cls==NULL) return;
    jclass classString=(jclass) (*pEnv)->NewGlobalRef(pEnv,obj);
    if(classString==NULL) return;
    jmethodID mid_getBytes=(*pEnv)->GetMethodID(pEnv,cls,"printStr","()V");
    if(mid_getBytes==NULL) return;
    (*pEnv)->CallVoidMethod(pEnv,obj,mid_getBytes);
}

JNIEXPORT jint JNICALL Java_kr_selfcontrol_exeemulator_dasm_isFinish(JNIEnv *pEnv, jobject obj) {

    if (isFinish())
        return 1;
    return 0;
}


JNIEXPORT jint JNICALL Java_kr_selfcontrol_exeemulator_dasm_getInputType(JNIEnv *pEnv,
                                                                         jobject obj) {
    return getInputType();
}

JNIEXPORT void JNICALL Java_kr_selfcontrol_exeemulator_dasm_inputChar(JNIEnv *pEnv, jobject obj,
                                                                      jstring jstr) {
    char *str = (*pEnv)->GetStringUTFChars(pEnv, jstr, 0);
    inputChar(str);
}

JNIEXPORT void JNICALL Java_kr_selfcontrol_exeemulator_dasm_inputFloat(JNIEnv *pEnv, jobject obj,
                                                                       jfloat jf) {
    inputFloat(jf);

}

JNIEXPORT void JNICALL Java_kr_selfcontrol_exeemulator_dasm_inputInt(JNIEnv *pEnv, jobject obj,
                                                                     jint ji) {
    inputInt(ji);
}

JNIEXPORT void JNICALL Java_kr_selfcontrol_exeemulator_dasm_printBufferClear(JNIEnv *pEnv,
                                                                             jobject obj) {
    printBufferClear();
}
