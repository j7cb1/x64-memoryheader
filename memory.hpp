#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <TlHelp32.h>

namespace memory
{
    HANDLE processhandle = nullptr;
    __int32 pid = 0;

    __int32 GetProcessId(std::string processname)
    {
        PROCESSENTRY32 Entry;
        Entry.dwSize = sizeof PROCESSENTRY32;

        HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (!Process32First(Snapshot, &Entry)) {
            return 0;
        }

        while (Process32Next(Snapshot, &Entry)) {
            if (strcmp(Entry.szExeFile, processname.c_str()) == 0) {
                return (__int32)Entry.th32ProcessID;
            }
        }

        return 0;
    }

    bool Setup(std::string targetname)
    {
        if (pid = GetProcessId(targetname) == 0) {
            printf("> failed to find target process\n");
            return false;
        }

        if ((processhandle = OpenProcess(PROCESS_ALL_ACCESS, false, pid)) == INVALID_HANDLE_VALUE) {
            printf("> invalid handle value to target process\n");
            return false;
        }

        return true;
    }

    bool ReadProcessMemoryWrapper(__int64 address, void* buffer, __int64 size) {
        return ReadProcessMemory(processhandle, (void*)address, buffer, size, nullptr);
    }

    bool WriteProcessMemoryWrapper(__int64 address, void* buffer, __int64 size) {
        return WriteProcessMemory(processhandle, (void*)address, buffer, size, nullptr);
    }

    template<typename Type>
    Type Read(__int64 address)
    {
        Type Buffer;
        ReadProcessMemoryWrapper(address, &Buffer, sizeof Type);

        return Buffer;
    }

    template<typename Type>
    Type ReadChain(__int64 address, std::vector<__int64> chain)
    {
        __int64 Current = address;
        for (int i = 0; i < chain.size() - 1; i++)
        {
            Current = memory::Read<__int64>(Current + chain[i]);
        }
        return memory::Read<Type>(Current + chain[chain.size() - 1]);
    }

    template<typename Type>
    bool Write(__int64 address, void* buffer)
    {
        return WriteProcessMemoryWrapper(address, buffer, sizeof Type);
    }

    std::string ReadString(__int64 address)
    {
        char Buffer[1024];
        ReadProcessMemoryWrapper(Read<__int64>(address), Buffer, 1024);
        return std::string(Buffer + sizeof Buffer / sizeof Buffer[0]);
    }

    __int64 GetModuleBase(std::string modulename)
    {
        HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (Snapshot != INVALID_HANDLE_VALUE) {

            MODULEENTRY32 modEntry;
            modEntry.dwSize = sizeof(modEntry);
            if (Module32First(Snapshot, &modEntry)) {

                do {
                    if (strcmp(modEntry.szModule, modulename.c_str()) == 0) {
                        return (__int64)modEntry.modBaseAddr;
                    }
                } while (Module32Next(Snapshot, &modEntry));
            }
        }

        return 0;
    }
}

#endif 