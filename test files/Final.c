#include "C:/Program Files/Java/jdk1.8.0_333/include/jni.h"
#include "C:/Program Files/Java/jdk1.8.0_333/include/win32/jni_md.h"
#include <windows.h>
#include <stdio.h>

JNIEXPORT jobjectArray JNICALL Java_SystemLogReader_readSystemLog(JNIEnv *env, jobject obj, jstring logName) {
    jobjectArray result;
    jclass cls;
    jmethodID constructor;
    jstring jsource, jdescription;
    HANDLE hEventLog;
    EVENTLOGRECORD *pevlr;
    DWORD dwBytesRead, dwBytesNeeded;
    char szBuffer[2048];
    int i, count = 0;

    // Convert the Java string to a C string
    const char *cLogName = (*env)->GetStringUTFChars(env, logName, NULL);
    if (cLogName == NULL) {
        return NULL;
    }

    // Open the event log
    hEventLog = OpenEventLog(NULL, cLogName);
    if (hEventLog == NULL) {
        printf("Failed to open the event log (error code %d).\n", GetLastError());
        return NULL;
    }

    // Count the number of event log records
    while (ReadEventLog(hEventLog, EVENTLOG_BACKWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                        0, szBuffer, sizeof(szBuffer), &dwBytesRead, &dwBytesNeeded)) {
        pevlr = (EVENTLOGRECORD *)szBuffer;

        while ((DWORD)((LPBYTE)pevlr - (LPBYTE)szBuffer) < dwBytesRead) {
            count++;

            pevlr = (EVENTLOGRECORD *)((LPBYTE)pevlr + pevlr->Length);
        }
    }

    // Allocate the array of objects
    cls = (*env)->FindClass(env, "EventLogRecord");
    if (cls == NULL) {
        return NULL;
    }
    constructor = (*env)->GetMethodID(env, cls, "<init>", "(ILjava/lang/String;Ljava/lang/String;)V");
    if (constructor == NULL) {
        return NULL;
    }
    result = (*env)->NewObjectArray(env, count, cls, NULL);
    if (result == NULL) {
        return NULL;
    }

    // Read the event log records and add them to the array
    i = 0;
    while (ReadEventLog(hEventLog, EVENTLOG_BACKWARDS_READ | EVENTLOG_SEQUENTIAL_READ,
                        0, szBuffer, sizeof(szBuffer), &dwBytesRead, &dwBytesNeeded)) {
        pevlr = (EVENTLOGRECORD *)szBuffer;

        while ((DWORD)((LPBYTE)pevlr - (LPBYTE)szBuffer) < dwBytesRead) {
            jsource = (*env)->NewStringUTF(env, (LPSTR)((LPBYTE)pevlr + sizeof(EVENTLOGRECORD)));
            jdescription = (*env)->NewStringUTF(env, (LPSTR)((LPBYTE)pevlr + pevlr->StringOffset));

            jobject eventLogRecord = (*env)->NewObject(env, cls, constructor, pevlr->EventID, jsource, jdescription);
            (*env)->SetObjectArrayElement(env, result, i, eventLogRecord);
            i++;

            (*env)->DeleteLocalRef(env, jsource);
            (*env)->DeleteLocalRef(env, jdescription);

            pevlr = (EVENTLOGRECORD *)((LPBYTE)pevlr + pevlr->Length);
        }
    }

    // Close the event log

    CloseEventLog(hEventLog);

    // Release the C string
    (*env)->ReleaseStringUTFChars(env, logName, cLogName);

    return result;
}

   
