#ifndef __INSTANCE_H__
#define __INSTANCE_H__

#include <Windows.h>

#define __MUTEX_NAME__ L"XPClockMutex"

void CheckInstance(void); // Checks if an instance of the application is running. If it is, it will ask the user what to do.

#endif // !__INSTANCE_H__
