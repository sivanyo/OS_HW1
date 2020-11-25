#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include "Commands.h"
#include "signals.h"

SmallShell &smash = SmallShell::getInstance();
extern bool gotQuit;

// TODO: remove this before sending
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

int main(int argc, char *argv[]) {
    if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-Z handler");
    }
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror("smash error: failed to set ctrl-C handler");
    }

    struct sigaction act;
    act.sa_handler = alarmHandler;
    act.sa_flags = SA_RESTART;

    int result = sigaction(SIGALRM, &act, 0);
    if (result == -1) {
        perror("smash error: failed to set alarm handler");
    }

    while (!gotQuit) {
        std::cout << smash.GetPrompt();
        std::string cmd_line;
        std::getline(std::cin, cmd_line);
        smash.executeCommand(cmd_line.c_str());
    }
    return 0;
}

#pragma clang diagnostic pop