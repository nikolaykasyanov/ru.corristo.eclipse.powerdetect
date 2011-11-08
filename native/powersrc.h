#include <jni.h>

#ifndef _powersrc_h
#define _powersrc_h
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *, void *);

/*
 * Class:     ru_corristo_eclipse_powerdetect_PowerSourceDetector
 * Method:    initialize
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_ru_corristo_eclipse_powerdetect_PowerSourceDetector_initialize
  (JNIEnv *, jclass);

/*
 * Class:     ru_corristo_eclipse_powerdetect_PowerSourceDetector
 * Method:    destroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_ru_corristo_eclipse_powerdetect_PowerSourceDetector_destroy
  (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
