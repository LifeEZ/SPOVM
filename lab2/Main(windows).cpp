#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include <vector>
#include <map>

HANDLE NewProcess(const int& ProcNumber, const std::string& ParentPath) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);

	char args[5];
	sprintf(args + 1, "%d", ProcNumber);
	args[0] = ' ';

	if (!CreateProcessA(
		ParentPath.c_str(),
		args,
		nullptr,
		nullptr,
		FALSE,
		NULL,
		nullptr,
		nullptr,
		&si,
		&pi)) {
		std::cout << "Failed creating process" << std::endl;
		exit(-1);
	}
	return pi.hProcess;
}

int main(int argc, char** argv) {
	if (argc > 1) {
		HANDLE WriteEvent = OpenEventA(
			EVENT_ALL_ACCESS,
			false,
			(std::string("WriteEvent_") + argv[argc - 1]).c_str());
		while (true) {
			WaitForSingleObject(WriteEvent, INFINITE);
			std::string a = ("pid ");
			for (char i: a) {
				printf("%c", i);
				Sleep(100);
			}
			for (size_t i = 0; i < strlen(argv[argc - 1]); i++) {
				printf_s("%c", argv[argc - 1][i]);
				Sleep(100);
			}
			printf_s("\n");
			ResetEvent(WriteEvent);
			while (WaitForSingleObject(WriteEvent, 0) == WAIT_OBJECT_0);
		}
	}

	std::vector<std::pair<HANDLE, HANDLE>> Children;//process,event
	Children.reserve(20);
	unsigned CurrentWriting = 0;
	bool ExitFlag = false;

	std::cout << "+. Add process" << std::endl;
	std::cout << "-. Kill last process" << std::endl;
	std::cout << "q. Exit" << std::endl;

	while (true) {
		while(_kbhit()) {
			switch (_getch()) {
			case 'q':
				while (!Children.empty()) {
					const std::pair<HANDLE, HANDLE> Child = Children.back();
					TerminateProcess(Child.first, EXIT_SUCCESS);
					CloseHandle(Child.second);
					CloseHandle(Child.first);
					Children.pop_back();
				}
				ExitFlag = true;
				break;
			case '+':
				if (Children.size() < 20) {
					HANDLE WriteEvent = CreateEventA(
						nullptr,
						TRUE,
						FALSE,
						(std::string("WriteEvent_") + std::to_string(Children.size() + 1)).c_str());
					HANDLE Process = NewProcess(static_cast<int>(Children.size()) + 1, argv[0]);
					Children.emplace_back(Process, WriteEvent);
				}
				break;
			case '-':
				if (!Children.empty()) {
					const std::pair<HANDLE, HANDLE> Child = Children.back();
					TerminateProcess(Child.first, EXIT_SUCCESS);
					CloseHandle(Child.second);
					CloseHandle(Child.first);
					Children.pop_back();
					if (Children.empty())
						std::cout << "\nNo more processes running. To add new one press \"+\" or \"q\" to exit" << std::endl;
				}
				else {
					std::cout << "\nNo more processes running. To add new one press \"+\" or \"q\" to exit" << std::endl;
				}
				break;
			default: ;
			}
		}
		if(ExitFlag) {
			break;
		}
		if (!Children.empty()) {
			if (++CurrentWriting >= Children.size())
				CurrentWriting = 0;
			SetEvent(Children[CurrentWriting].second);
			while (WaitForSingleObject(Children[CurrentWriting].second, 0) == WAIT_OBJECT_0);
		}
	}
}