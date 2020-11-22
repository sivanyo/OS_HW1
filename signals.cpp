#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

extern SmallShell &smash;

using namespace std;

void ctrlZHandler(int sig_num) {
    signal(SIGSTOP, &ctrlCHandler);
    std::cout << "smash: got ctrl-Z" << std::endl;
    int fgPid = smash.getFgPid();
    if (fgPid != 0) {
        int jobID = smash.getJobsReference()->getJobIdByProcessId(fgPid);
        smash.getJobs().getJobsMap().find(jobID)->second.setStopped(true);
        if (kill(fgPid, SIGSTOP) == -1) {
            perror("smash error: kill failed");
            return;
        }
        std::cout << "smash: process " << fgPid << " was stopped" << std::endl;
    }

}

void ctrlCHandler(int sig_num) {
    // reconnect handler for future signals
    signal(SIGINT, &ctrlCHandler);
    std::cout << "smash: got ctrl-C" << std::endl;
    // Send the signal to the correct process
    int jobId = smash.getJobs().getCurrentMaxJobId();
    if (jobId != 0) {
        int procPid = smash.getJobs().getJobsMap().find(jobId)->second.getPid();
        int result = kill(procPid, SIGINT);
        if (result == -1) {
            perror("smash error: kill failed");
            return;
        }
        std::cout << "smash: process " << procPid << " was killed" << std::endl;
        //smash.getJobsReference()->removeJobById(jobId);
    }
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
}

