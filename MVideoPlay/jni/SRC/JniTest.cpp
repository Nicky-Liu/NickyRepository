#include <jni.h>
#include <stdio.h>
#include <String>
#include <stdlib.h>
#include <android/log.h>
#include "Header/AudioTest.h"

#ifndef  LOG_TAG
#define  LOG_TAG    "log_Test"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

//指定要注册的类
static const char *classPathName = "com/example/studyffmpeg/NativeClass";

/*
 * ffmpeg 的操作
 * */
JNIEXPORT void JNICALL ffmpegTest(JNIEnv* env, jobject thiz)
{
	//decode_audio();
}

/**
 *
 *从jni返回字符串到java
 *
 * */
JNIEXPORT jstring JNICALL printHello(JNIEnv* env, jobject thiz)
{
	/*	av_register_all();
	 avcodec_register_all();
	 AVFormatContext *pFormatCtx = NULL;
	 if (avformat_open_input(&pFormatCtx, "/storage/emulated/0/a.mp4", NULL,
	 NULL) < 0)
	 {
	 LOGI("打开失败！");
	 }
	 else
	 {
	 LOGI("打开成功！");
	 }*/

	LOGI("Hello World From libhelloworld.so!");
	return env->NewStringUTF("Hello World!你   某某某某某");
}

/**
 * 大小写进行转换
 *
 * */
JNIEXPORT jstring JNICALL CopyString(JNIEnv* env, jobject thiz, jstring strOrg)
{
	const char *str;
	jstring desString;
	int index;
	char ptr[10];

	str = (char*) env->GetStringUTFChars(strOrg, NULL);
	if (str == NULL)
	{
		LOGI("Out Of MemoryError already thrown!");
		return NULL;
	}
	else
	{
		LOGI("The string from java is: %s\n The lenth of the string is:%d ",str,sizeof(str));
	}

	for (index = 0; index < sizeof(str); index++)
	{

		if (str[index] >= 'a' && str[index] <= 'z')
		{
			ptr[index] = str[index] - 32;
		}
		else
		{
			ptr[index] = str[index] + 32;
		}
	}
	ptr[index] = '\0';

	LOGI("The string  is: %s",ptr);

	desString = env->NewStringUTF(ptr);
	env->ReleaseStringUTFChars(strOrg, str);
	return desString;
}

/*
 * 数组求和函数1
 * */
JNIEXPORT jint JNICALL getSumArray1(JNIEnv *env, jobject obj, jintArray arr)
{
	LOGI("getSumArray1 is called!");
	jint buf[10];
	jint i, sum = 0;
	env->GetIntArrayRegion(arr, 0, 10, buf);

	for (i = 0; i < 10; i++)
	{
		sum += buf[i];
	}
	return sum;
}

/**
 * 数组求和函数2
 */
JNIEXPORT jint JNICALL getSumArray2(JNIEnv* env, jobject thiz, jintArray arry)
{
	LOGI("getSumArray2 is called!");
	jint *carr;
	jint i, sum = 0;
	jsize lenth;
	carr = env->GetIntArrayElements(arry, NULL);
	if (carr == NULL)
	{
		return 0; /* exception occurred */
	}
	lenth = env->GetArrayLength(arry);
	for (i = 0; i < lenth; i++)
	{
		sum += carr[i];
	}
	env->ReleaseIntArrayElements(arry, carr, 0);
	return sum;
}

/*
 * 对象数组访问
 * */
JNIEXPORT jobjectArray JNICALL
initInt2DArray(JNIEnv *env, jclass cls, int size)
{
	jobjectArray result;
	int i;
	jclass intArrCls = env->FindClass("[I");
	if (intArrCls == NULL)
	{
		return NULL; /* exception thrown */
	}
	result = env->NewObjectArray(size, intArrCls, NULL);
	if (result == NULL)
	{
		return NULL; /* out of memory error thrown */
	}
	for (i = 0; i < size; i++)
	{
		jint tmp[256]; /* make sure it is large enough! */
		int j;
		jintArray iarr = env->NewIntArray(size);
		if (iarr == NULL)
		{
			return NULL; /* out of memory error thrown */
		}
		for (j = 0; j < size; j++)
		{
			tmp[j] = i + j;
		}
		env->SetIntArrayRegion(iarr, 0, size, tmp);
		env->SetObjectArrayElement(result, i, iarr);
		env->DeleteLocalRef(iarr);
	}
	return result;
}

/*
 * c++访问java中的数据，并进行改变
 *
 * */
JNIEXPORT void JNICALL
accessField(JNIEnv *env, jobject obj)
{
	jfieldID fid;
	jstring jstr;
	const char *str;

	jclass cls = env->GetObjectClass(obj);
	LOGI("In C:\n");
	/* Look for the instance field s in cls */
	fid = env->GetFieldID(cls, "s", "Ljava/lang/String;");
	if (fid == NULL)
	{
		return; /* failed to find the field */
	}
	jstr = (jstring) env->GetObjectField(obj, fid);
	str = env->GetStringUTFChars(jstr, NULL);
	if (str == NULL)
	{
		return; /* out of memory */
	}
	LOGI(" NativeClass.s = \"%s\"\n", str);
	env->ReleaseStringUTFChars(jstr, str);
	jstr = env->NewStringUTF("123456");
	if (jstr == NULL)
	{
		return; /* out of memory */
	}
	env->SetObjectField(obj, fid, jstr);
}

/*
 * C++ 调用java中的方法
 * */
JNIEXPORT void JNICALL
callJFMethod(JNIEnv *env, jobject obj)
{
	jclass cls = env->GetObjectClass(obj);
	jmethodID mid = env->GetMethodID(cls, "callBack", "()V");
	if (mid == NULL)
	{
		return; /* method not found */
	}
	LOGI("In C\n");
	env->CallVoidMethod(obj, mid);
}
/**
 * 方法对应表
 */
static JNINativeMethod methodsTab[] =
{
{ "StringFromJNI", "()Ljava/lang/String;", (void*) printHello },
{ "StringCopyFrom", "(Ljava/lang/String;)Ljava/lang/String;",
		(void*) CopyString },
{ "ffmpegTest", "()V", (void*) ffmpegTest },
{ "sumArray1", "([I)I", (void*) getSumArray1 },
{ "sumArray2", "([I)I", (void*) getSumArray2 },
{ "initInt2DArray", "(I)[[I", (void*) initInt2DArray },
{ "accessField", "()V", (void*) accessField },
{ "nativeMethod", "()V", (void*) callJFMethod } };

//初始化函数
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jclass clazz;

	//获取JNI环境对象
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		LOGE("ERROR: GetEnv failed\n");
		return JNI_ERR;
	}

	//注册本地方法,Load目标类
	clazz = env->FindClass(classPathName);
	if (clazz == NULL)
	{
		LOGE("Native registration unable to find class '%s'", classPathName);
		return JNI_ERR;
	}

	//注册本地方法
	if (env->RegisterNatives(clazz, methodsTab, sizeof(methodsTab)
			/ sizeof(methodsTab[0])) < 0)
	{
		LOGE("ERROR: MediaPlayer native registration failed\n");
		return JNI_ERR;
	}

	//返回版本号
	return JNI_VERSION_1_4;
}
