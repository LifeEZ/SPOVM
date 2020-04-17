#include <iostream>
#include <csignal>
#include <vector>
#include <unistd.h>
#include "conio.h"

#pragma clang diagnostic ignored "-Wmissing-noreturn"
#define FROM_CHILD SIGUSR1
#define FROM_PARENT SIGUSR2

bool writable = true;
unsigned currentWritingIndex = 0;
std::vector<pid_t> pidsVector;

void signalHandler(int signal) {
    switch (signal) {
        case FROM_CHILD:
            if (writable) {
                writable = false;
            } else {
                if (++currentWritingIndex >= pidsVector.size())
                    currentWritingIndex = 0;
            }
            kill(pidsVector.at(currentWritingIndex), FROM_PARENT);
            break;

        case FROM_PARENT:
            writable = true;
            break;
        default:;
    }
}

void child() {
    std::string pid = std::to_string(getpid());
    while (true) {

        writable = false;
        kill(getppid(), FROM_CHILD);
        while (!writable) {
            sleep(1);
        }
        for (char i : pid) {
            printf("%c", i);
            usleep(10000);
        }
        printf("\n");
    }
}

int main() {
    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sigaction(FROM_PARENT, &sa, nullptr);
    sigaction(FROM_CHILD, &sa, nullptr);
    //int max = 10;
    std::cout << "\"+\" : create process\n\"-\" : delete last process\n\"q\" : close program" << std::endl;
    while (true) {
        while (kbhit()) {
            switch (getch_(0)) {
                case '+':
                    //if (pidsVector.size() >= max) {
                        //break;
                    //}
                    switch (int pid = fork()) {
                        case -1:
                            std::cout << "Error." << std::endl;
                            exit(1);
                        case 0:
                            child();
                            break;
                        default:
                            pidsVector.push_back(pid);
                            break;
                    }
                    break;
                case '-':
                    if (!pidsVector.empty()) {
                        kill(pidsVector.at(pidsVector.size() - 1), SIGTERM);
                        pidsVector.pop_back();
                        if (pidsVector.empty()) {
                            std::cout << R"(No processes running, enter "+" to add new one or "q" to exit the program.)"
                                      << std::endl;
                        }
                    } else {
                        std::cout << R"(No processes running, enter "+" to add new one or "q" to exit the program.)" << std::endl;
                    }
                    break;
                case 'q':
                    while (!pidsVector.empty()) {
                        kill(pidsVector.at(pidsVector.size() - 1), SIGTERM);
                        pidsVector.pop_back();
                    }
                    exit(0);
            }
        }
    }
}