#include "powersrc.h"

#include <stdio.h>
#include <pthread.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/ps/IOPSKeys.h>
#include <IOKit/ps/IOPowerSources.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	AC,
	BATTERY
} POWER_SOURCE;

// java related stuff
static JavaVM *jvm;
static jclass listenerClass;
static jmethodID listenerMethod_id;

static POWER_SOURCE lastPowerSource;

void powerSourceChanged(void *context);
void * thread_start(void *);
POWER_SOURCE currentPowerSource(void);
void notify();

jint GetJNIEnv(JNIEnv **env, int *mustDetach)
{
	jint getEnvErr = JNI_OK;
	*mustDetach = 0;
	if (jvm) {
		getEnvErr = (*jvm)->GetEnv(jvm, (void **)env, JNI_VERSION_1_6);
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

	listenerClass = (*env)->FindClass(env, "ru/corristo/eclipse/powerdetect/PowerSourceListener");

	if (listenerClass == NULL) {
		printf("Can not find PowerSourceListener class\n");
		return;
	}

	// make listenerClass global ref
	listenerClass = (*env)->NewGlobalRef(env, listenerClass);
	if (listenerClass == NULL) {
		printf("Can not create global ref, possibly OutOfMemoryError\n");
		return;
	}

	listenerMethod_id = (*env)->GetStaticMethodID(env, listenerClass, "powerSourceChanged", "(Z)V");

	if (listenerMethod_id == NULL) {
		printf("Can not find method PowerSourceListener.powerSourceChanged\n");
		return;
	}

	// get initial value
	lastPowerSource = currentPowerSource();
	notify(lastPowerSource);

	// create new thread
	pthread_t thread;
	int rc = pthread_create(&thread, NULL, &thread_start, NULL);
}

JNIEXPORT void JNICALL Java_ru_corristo_eclipse_powerdetect_PowerSourceDetector_destroy(JNIEnv * env, jclass cls)
{
	printf("destroy called\n");

	// kill thread there?

	// remove global ref if exists
	if (listenerClass == NULL) {
		(*env)->DeleteGlobalRef(env, listenerClass);
	}
}

void notify(POWER_SOURCE powerSource) {
	if (listenerClass == NULL || listenerMethod_id == NULL) {
		printf("Class not initialized\n");
		return;
	}

	JNIEnv *env;
	int shouldDetach = 0;

	if (GetJNIEnv(&env, &shouldDetach) != JNI_OK) {
		printf("can not attach to JVM\n");
		return;
	}

	(*env)->CallStaticVoidMethod(env, listenerClass, listenerMethod_id, powerSource);

	if (shouldDetach) {
		(*jvm)->DetachCurrentThread(jvm);
	}
}

void powerSourceChanged(void *context) {
	printf("power source notification received\n");

	POWER_SOURCE value = currentPowerSource();

	if (value != lastPowerSource) { // check if power source actually changed
		lastPowerSource = value;
		notify(value);
	}
}

void *thread_start(void *arg) {
	printf("starting thread\n");

	CFRunLoopSourceRef CFrls;
    
    CFrls = IOPSNotificationCreateRunLoopSource(powerSourceChanged, NULL);
    if(CFrls) {
    	printf("adding run loop source\n");

    	CFRunLoopRef r = CFRunLoopGetCurrent();
        CFRunLoopAddSource(CFRunLoopGetCurrent(), CFrls,
                           kCFRunLoopDefaultMode);
        CFRelease(CFrls);
    }

    CFRunLoopRun();

    return NULL;
}

POWER_SOURCE currentPowerSource() {
	CFTypeRef blob = IOPSCopyPowerSourcesInfo();
	CFArrayRef list = IOPSCopyPowerSourcesList(blob);

	POWER_SOURCE result = BATTERY;

	int i = 0;

	CFIndex sourceCount = CFArrayGetCount(list);

	for (i = 0; i < sourceCount; i++) {
		CFStringRef powerSourceState;

		CFTypeRef source = (CFTypeRef)CFArrayGetValueAtIndex(list, i);

		CFDictionaryRef dict = (CFDictionaryRef)IOPSGetPowerSourceDescription(blob, source);
		
		powerSourceState = CFDictionaryGetValue(dict, CFSTR(kIOPSPowerSourceStateKey));

		if(CFEqual(powerSourceState, CFSTR(kIOPSACPowerValue))) {
			result = AC;
			break;
		}
	}

	CFRelease(blob);
	CFRelease(list);

	return result;
}

#ifdef __cplusplus
}
#endif