#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <vector>
#include <string>

void error(const char* error_title, const char* error_message)
{
    MessageBox(NULL, error_message, error_title, NULL);
    exit(-1);
}

bool file_exists(const char* file_name)
{
    return GetFileAttributes(file_name) != INVALID_FILE_ATTRIBUTES;
}

int main()
{
    const char* dll_name = "?";  // Replace with the actual path to your DLL.

    if (!file_exists(dll_name))
    {
        error("file_exists", "File Doesn't Exist");
    }


        std::cout << R"(
 _________.__                                      .__    ________  .____    .____      .___            __               __
/   _____/|  |______________  ______   ____   ____ |  |   \______ \ |    |   |    |     |   | ____     |__| ____   _____/  |_  ___________
\_____  \ |  |  \_  __ \__  \ \____ \ /    \_/ __ \|  |    |    |  \|    |   |    |     |   |/    \    |  |/ __ \_/ ___\   __\/  _ \_  __ \
 /        \|   Y  \  | \// __ \|  |_> >   |  \  ___/|  |__  |    `   \    |___|    |___  |   |   |  \   |  \  ___/\  \___|  | (  <_> )  | \/
/_______  /|___|  /__|  (____  /   __/|___|  /\___  >____/ /_______  /_______ \_______ \ |___|___|  /\__|  |\___  >\___  >__|  \____/|__|
        \/      \/           \/|__|        \/     \/               \/        \/       \/          \/\______|    \/     \/     
)" << std::endl;

        // Display the menu and get the user's choice
        while (true)
        {

        std::cout << "1. Process List" << std::endl;
        std::cout << "2. Quit Injector" << std::endl;
        std::cout << "Please make a choice (1 or 2): ";
        std::string choice;
        std::cin >> choice;

        if (choice == "1")
        {
            DWORD process_ids[1024];
            DWORD bytes_needed;
            if (!EnumProcesses(process_ids, sizeof(process_ids), &bytes_needed))
            {
                error("EnumProcesses", "Failed to enumerate processes");
            }

            int num_processes = bytes_needed / sizeof(DWORD);

            std::vector<std::string> process_names;

            for (int i = 0; i < num_processes; i++)
            {
                if (process_ids[i] != 0)
                {
                    HANDLE h_process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_ids[i]);
                    if (h_process)
                    {
                        char exe_path[MAX_PATH];
                        if (GetModuleFileNameExA(h_process, 0, exe_path, MAX_PATH))
                        {
                            process_names.push_back(exe_path);
                        }
                        CloseHandle(h_process);
                    }
                }
            }

            if (process_names.empty())
            {
                error("No Running Processes", "No running processes found.");
            }

            std::cout << "Select a process to inject the DLL into:" << std::endl;

            for (int i = 0; i < process_names.size(); i++)
            {
                std::cout << i + 1 << ": " << process_names[i] << std::endl;
            }

            int selected_process;
            std::cout << "Enter the number of the process to inject the DLL into: ";
            std::cin >> selected_process;

            if (selected_process >= 1 && selected_process <= process_names.size())
            {
                const char* selected_exe_path = process_names[selected_process - 1].c_str();
                HANDLE h_process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_ids[selected_process - 1]);
                if (h_process)
                {
                    void* allocated_memory = VirtualAllocEx(h_process, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                    if (allocated_memory)
                    {
                        if (WriteProcessMemory(h_process, allocated_memory, dll_name, strlen(dll_name) + 1, nullptr))
                        {
                            HANDLE h_thread = CreateRemoteThread(h_process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA), allocated_memory, 0, nullptr);
                            if (h_thread)
                            {
                                MessageBox(0, "Successfully Injected!", "Success", 0);
                            }
                            else
                            {
                                error("CreateRemoteThread", "Failed to create remote thread");
                            }
                        }
                        else
                        {
                            error("WriteProcessMemory", "Failed to write process memory");
                        }
                        VirtualFreeEx(h_process, allocated_memory, 0, MEM_RELEASE);
                    }
                    else
                    {
                        error("VirtualAllocEx", "Failed to allocate memory");
                    }
                    CloseHandle(h_process);
                }
            }
            else
            {
                error("Invalid Selection", "Invalid process selection.");
            }
        }
        else if (choice == "2")
        {
            break; // Exit the program
        }
        else
        {
            std::cout << "Invalid choice. Please select 1 for Process List or 2 to Quit." << std::endl;
        }
    }

    return 0;
}
