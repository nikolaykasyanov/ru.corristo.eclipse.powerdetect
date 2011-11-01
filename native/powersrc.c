#include "powersrc.h"

#include <stdio.h>
#include <pthread.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>

#ifdef __cplusplus
extern "C" {
#endif

static JavaVM *jvm;
static jclass jcls;
static jmethodID powerSourceChanged_id;

void powerSourceChanged(void *context);
void * thread_start(void *);
int isOnBattery(void);

jint GetJNIEnv(JNIEnv **env, int *mustDetach)
{
	jint getEnvErr = JNI_OK;
	*mustDetach = 0;
	if (jvm) {
		getEnvErr = (*jvm)->GetEnv(jvm, (void **)env, JNI_VERSION_1_4);
		if (getEnvErr == JNI_EDETACHED) {
			getEnvErr = (*jvm)->AttachCurrentThread(jvm, (void **)env, NULL);
			if (getEnvErr == JNI_OK) {
				*mustDetach = 1;
			}
		}
	}
	return getEnvErr;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
	jvm = vm;

	return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL Java_ru_corristo_eclipse_powerdetect_PowerSourceDetector_initialize(JNIEnv * env, jclass cls)
{
	printf("initialize called\n");

	jcls = (*env)->FindClass(env, "ru/corristo/eclipse/powerdetect/PowerSourceListener");

	if (jcls == 0) {
		printf("Can not find NativeInterop class\n");
		return;
	}

	powerSourceChanged_id = (*env)->GetStaticMethodID(env, jcls, "powerSourceChanged", "(Z)V");

	if (powerSourceChanged_id == 0) {
		printf("Can not find method powerSourceChanged\n");
		return;
	}

	// create new thread
	pthread_t thread;
	int rc = pthread_create(&thread, NULL, &thread_start, NULL);
}

JNIEXPORT void JNICALL Java_ru_corristo_eclipse_powerdetect_PowerSourceDetector_destroy(JNIEnv * env, jclass cls)
{
	printf("destroy called\n");

	// kill thread there?
}

void powerSourceChanged(void *context) {

	int value;

	printf("power source changed\n");

	if (jcls == 0 || powerSourceChanged_id == 0) {
		printf("Class not initialized\n");
		return;
	}

	JNIEnv *env;
	int shouldDetach = 0;

	if (GetJNIEnv(&env, &shouldDetach) != JNI_OK) {
		printf("can not attach to JVM\n");
		return;
	}

	value = isOnBattery();

	(*env)->CallStaticVoidMethod(env, jcls, powerSourceChanged_id, value);

	if (shouldDetach) {
		(*jvm)->DetachCurrentThread(jvm);
	}
}

void *thread_start(void *arg) {
	printf("starting thread\n");

	CFRunLoopSourceRef CFrls;
    
    CFrls = IOPSNotificationCreateRunLoopSource(&powerSourceChanged, NULL);
    if(CFrls) {
    	printf("adding run loop source\n");

    	CFRunLoopRef r = CFRunLoopGetCurrent();

    	if (r == 0) {
    		printf("CFRunLoopGetCurrent failed\n");
    	}

        CFRunLoopAddSource(CFRunLoopGetCurrent(), CFrls,
                           kCFRunLoopDefaultMode);
        CFRelease(CFrls);
    }

    CFRunLoopRun();

    return NULL;
}

int isOnBattery() {
	CFTypeRef blob = IOPSCopyPowerSourcesInfo();
	CFArrayRef list = IOPSCopyPowerSourcesList(blob);

	int onBattery = 1;

	int i = 0;

	CFIndex sourceCount = CFArrayGetCount(list);

	for (i = 0; i < sourceCount; i++) {
		CFStringRef powerSourceState;

		CFTypeRef source = (CFTypeRef)CFArrayGetValueAtIndex(list, i);

		CFDictionaryRef dict = (CFDictionaryRef)IOPSGetPowerSourceDescription(blob, source);
		
		powerSourceState = CFDictionaryGetValue(dict, CFSTR(kIOPSPowerSourceStateKey));

		if(CFEqual(powerSourceState, CFSTR(kIOPSACPowerValue))) {
			onBattery = 0;
			break;
		}
	}

	CFRelease(blob);
	CFRelease(list);

	return onBattery;
}

#ifdef __cplusplus
}
#endif