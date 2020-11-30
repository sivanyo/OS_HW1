#include <iostream>
#include <signal.h>
#include <sys/wait.h>
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
    //int jobId = smash.getJobs().getCurrentMaxJobId();
    int fgPid = smash.getFgPid();
    //if (jobId != 0) {
    if (fgPid != 0) {
        //int procPid = smash.getJobs().getJobsMap().find(jobId)->second.getPid();
        //int result = kill(procPid, SIGINT);
        int result = killpg(fgPid, SIGKILL);
        if (result == -1) {
            perror("smash error: kill failed");
            return;
        }
        //std::cout << "smash: process " << procPid << " was killed" << std::endl;
        std::cout << "smash: process " << fgPid << " was killed" << std::endl;
        //smash.getJobsReference()->removeJobById(jobId);
    }
}

void alarmHandler(int sig_num) {
    std::cout << "smash: got an alarm" << std::endl;
    time_t now = time(nullptr);
    int alarmId = smash.getAlarmsReference()->getAlarmIdOfExpiredAlarm(now);
    // This if is NOT called if no entry matches the current alarm, meaning this process was run in
    // the foreground and finished without issues before the alarm was called, therefore we deleted his
    // alarm entry after execution finished
    if (alarmId != -1) {
        // We have a real alarmId, meaning one of the following could have happend:
        // the normal case (fg): the command ran in the foreground but timed out before it finished
        // the normal case (bg): the command ran in the background but timed out before it finished
        // the proc end case (bg): the command ran in the background and finished before timeout was called
        // we need to get the process ID and wait for it, if it is zombie then we need to just remove the alarm from the alarms list and the job from the job list
        int pid = smash.getAlarms().getAlarmsMap().find(alarmId)->second.getRealPid();
        int jobId = smash.getAlarms().getAlarmsMap().find(alarmId)->second.getJobId();
        //int timedOutProcPid = smash.getJobs().getJobsMap().find(alarmId)->second.getPid();
        int status;
        int waitResult = waitpid(pid, &status, WNOHANG);
        if (waitResult != 0) {
            // The real command finished processing before timeout was called, just need to remove job and alarm
            smash.getJobsReference()->removeJobById(jobId);
            smash.getAlarmsReference()->removeAlarmById(alarmId);
            smash.getAlarmsReference()->updateMaxAlarmId();
        } else {
            // The real command did not finish processing, meaning it has now timed out and needs to be killed
            int result = killpg(pid, SIGINT);
            if (result == -1) {
                perror("smash error: kill failed");
                return;
            }
            std::cout << "smash: " << smash.getAlarms().getAlarmsMap().find(alarmId)->second.getOriginalCommand() << " timed out!" << std::endl;
            if (smash.getJobs().getJobsMap().find(jobId)->second.isBackground() || smash.getJobs().getJobsMap().find(jobId)->second.isStopped()) {
                smash.getJobsReference()->removeJobById(jobId);
            }
            smash.getAlarmsReference()->removeAlarmById(alarmId);
            smash.getAlarmsReference()->updateMaxAlarmId();
        }
    }
    // TODO: after finish handling the current timed out command, a new alarm should be set for the next command
    if (smash.getAlarmsReference()->getMaxId() != 0) {
        // There are other alarms, so we need to schedule the next one
        smash.getAlarmsReference()->scheduleNextAlarm(now);
    }
}

