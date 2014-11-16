#ifndef LARGE_PAGES_HPP_
#define LARGE_PAGES_HPP_

// To enable Large Pages in Windows do the following:
// 1. Run gpedit.msc (or search for "Group Policy").
// 2. Under "Computer Configuration", "Windows Settings", "Security Settings", "Local Policies" click on "User Rights Assignment".
// 3. In the right panel double-click the option "Lock Pages in Memory".
// 4. Click on "Add User or Group" and add either the current account or "Everyone".
// 5. Restart the computer
// The program must also be run with administrative rights. 
// If some other program runs this program then that program must be run with the same rights. The alternative is to disable UAC.

// To enable Large Pages in Linux do the following:
// 1. In terminal type "cat /proc/meminfo" and search for HugePages_Total and Hugepagesize
// 2. If the value of HugePages_Total * Hugepagesize igreater than or equal to the amount of memory you want to allocate you are done
// 3. Otherwise do "sudo sh -c "echo xxx > /proc/sys/vm/nr_hugepages"" where xxx is the desired amount of large pages(choose a number so that you get enough).

#include <cstdint>
#include <iostream>
#include <cstdio>
#ifdef _WIN32
#define NOMINMAX
#include <tchar.h>
#include <windows.h>
#else
#include <cstring>
#include <cstdlib>
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

class LargePages
{
public:
    static void initialize()
    {
        allowedToUse = false;
        inUse = false;
#ifndef _WIN32
        num = -1;
#endif
    }

    static void* malloc(size_t size, size_t alignment)
    {
        void* memory = nullptr;
        inUse = false;

        if (allowedToUse)
        {
#ifdef _WIN32
            memory = VirtualAlloc(NULL, size, MEM_LARGE_PAGES | MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (memory)
            {
                inUse = true;
            }
            else
            {
                displayError(TEXT("VirtualAlloc"), GetLastError());
                memory = _aligned_malloc(size, alignment);
            }
#else
            num = shmget(IPC_PRIVATE, size, IPC_CREAT | SHM_R | SHM_W | SHM_HUGETLB);
            if (num == -1)
            {
                std::cout << "ERROR: API     = shmget" << std::endl;
                std::cout << "       errno   = " << errno << std::endl;
                std::cout << "       message = " << strerror(errno) << std::endl;
                posix_memalign(&memory, alignment, size);
            }
            else
            {
                inUse = true;
                memory = shmat(num, NULL, 0x0);
            }
#endif
        }
        else
        {
#ifdef _WIN32
            memory = _aligned_malloc(size, alignment);
#else
            posix_memalign(&memory, alignment, size);
#endif
        }

        return memory;
    }

    static void free(void* memory)
    {
        if (!memory)
        {
            return;
        }
        if (!inUse)
        {
#ifdef _WIN32
            _aligned_free(memory);
#else
            std::free(memory);
#endif
            return;
        }
#ifdef _WIN32
        VirtualFree(memory, 0, MEM_RELEASE);
#else
        shmdt(memory);
        shmctl(num, IPC_RMID, NULL);
#endif
    }

    static void setAllowedToUse(bool allowed)
    {
        allowedToUse = allowed;
        changeLargePagePrivileges(allowed);
    }
private:
    static bool allowedToUse;
    static bool inUse;
#ifndef _WIN32
    static int num;
#endif

#ifdef _WIN32
	static void displayError(TCHAR* pszAPI, DWORD dwError)
	{
		LPVOID lpvMessageBuffer;

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwError,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpvMessageBuffer, 0, NULL);

		//... now display this string
		_tprintf(TEXT("ERROR: API        = %s\n"), pszAPI);
		_tprintf(TEXT("       error code = %d\n"), dwError);
		_tprintf(TEXT("       message    = %s\n"), lpvMessageBuffer);

		// Free the buffer allocated by the system
		LocalFree(lpvMessageBuffer);
	}
#endif

    static void changeLargePagePrivileges(bool enabled)
    {
#ifdef _WIN32
        HANDLE           hToken;
        TOKEN_PRIVILEGES tp;

        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
            displayError(TEXT("OpenProcessToken"), GetLastError());

        // Get the luid.
        if (!LookupPrivilegeValue(NULL, TEXT("SeLockMemoryPrivilege"), &tp.Privileges[0].Luid))
            displayError(TEXT("LookupPrivilegeValue"), GetLastError());

        tp.PrivilegeCount = 1;

        // Enable or disable privilege.
        tp.Privileges[0].Attributes = (enabled ? SE_PRIVILEGE_ENABLED : 0);
        auto status = AdjustTokenPrivileges(hToken, FALSE, &tp, 0, (PTOKEN_PRIVILEGES)NULL, 0);

        // It is possible for AdjustTokenPrivileges to return TRUE and still not succeed.
        // So always check for the last error value.
        auto error = GetLastError();
        if (!status || (error != ERROR_SUCCESS))
            displayError(TEXT("AdjustTokenPrivileges"), GetLastError());

        if (!CloseHandle(hToken))
            displayError(TEXT("CloseHandle"), GetLastError());
#endif
    }
};

#endif
