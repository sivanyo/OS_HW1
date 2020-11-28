#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <utility>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <sys/fcntl.h>
#include <fstream>
#include "Commands.h"
#include "Utils.h"

extern SmallShell &smash;
bool gotQuit = false;

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cerr << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cerr << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

#define DEBUG_PRINT cerr << "DEBUG: "

#define EXEC(path, arg) \
  execvp((path), (arg));

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() : jobs(JobsList()) {
// TODO: add your implementation
    pid = getpid();
}

SmallShell::~SmallShell() {
// TODO: add your implementation
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {
    // TODO: check if parsing fails for commands with a space in front of them and fix
    string command = string(cmd_line);
    bool background = _isBackgroundComamnd(cmd_line);
    bool redirectOutput = Utils::isRedirectionCommand(command);
    bool redirectAppend = Utils::isRedirectionCommandWithAppend(command);
    bool pipe = Utils::isPipeout(command);
    bool pipeErr = Utils::isPipeErr(command);
    if (redirectOutput || redirectAppend) {
        bool append = false;
        if (redirectAppend) {
            append = true;
        }
        return new RedirectionCommand(cmd_line, append, background);
    } else if (pipe || pipeErr) {
        bool err = false;
        if (pipeErr) {
            err = true;
        }
        return new PipeCommand(cmd_line, err);
    } else if (command.find("chprompt") == 0) {
        return new ChangePromptCommand(cmd_line);
    } else if (command.find("ls") == 0) {
        return new ListDirectoryFilesCommand(cmd_line);
    } else if (command.find("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (command.find("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    } else if (command.find("cd") == 0) {
        return new ChangeDirCommand(cmd_line);
    } else if (command.find("jobs") == 0) {
        return new JobsCommand(cmd_line, smash.getJobsReference());
    } else if (command.find("kill") == 0) {
        return new KillCommand(cmd_line);
    } else if (command.find("fg") == 0) {
        return new ForegroundCommand(cmd_line);
    } else if (command.find("bg") == 0) {
        return new BackgroundCommand(cmd_line);
    } else if (command.find("quit") == 0) {
        return new QuitCommand(cmd_line);
        /*
         * This part should include certain boolean flags for special commands (maybe should be above internal commands ifs)
         */
    } else if (command.find("timeout") == 0) {
        return new TimeoutCommand(cmd_line, background);
    } else if (command.find("cp") == 0) {
        return new CopyCommand(cmd_line, background);
    } else {
        if (command.empty()) {
            return nullptr;
        }
        // External command
        return new ExternalCommand(cmd_line, background);
    }
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command *cmd = CreateCommand(cmd_line);
    if (cmd) {
        if (cmd->isExternal()) {
            cmd->execute();
        } else {
            cmd->execute();
            delete cmd;
        }
    }
}

const string &SmallShell::getCurrDir() const {
    return curr_dir;
}

void SmallShell::setLastDir(string lastDir) {
    last_dir = lastDir;
}

void SmallShell::setCurrDir(string currDir) {
    curr_dir = currDir;
}

const string &SmallShell::getLastDir() const {
    return last_dir;
}


string SmallShell::GetPrompt() {
    return prompt;
}

void SmallShell::SetPrompt(string nPrompt) {
    prompt = nPrompt;
}

int SmallShell::GetPid() {
    return pid;
}

const JobsList &SmallShell::getJobs() const {
    return jobs;
}

JobsList *SmallShell::getJobsReference() {
    return &jobs;
}

void SmallShell::setJobs(JobsList &jobs) {
    SmallShell::jobs = jobs;
}

int SmallShell::getFgPid() const {
    return fgPid;
}

void SmallShell::setFgPid(int fgPid) {
    SmallShell::fgPid = fgPid;
}

const AlarmList &SmallShell::getAlarms() const {
    return alarms;
}

AlarmList *SmallShell::getAlarmsReference() {
    return &alarms;
}

void ChangePromptCommand::execute() {
    if (arguments.empty()) {
        smash.SetPrompt("smash> ");
    } else {
        string nPrompt = arguments[0];
        nPrompt += "> ";
        smash.SetPrompt(nPrompt);
    }
}

ChangePromptCommand::ChangePromptCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
}

Command::Command(const char *cmd_line) {
    int len = strlen(cmd_line);
    char *nCom = new char[len + 1];
    strcpy(nCom, cmd_line);
    commandLine = nCom;
    vector<string> split = Utils::stringToWords(commandLine);
    baseCommand = split[0];
    for (int i = 1; i < split.size(); ++i) {
        arguments.push_back(split[i]);
    }
}

bool Command::isExternal() const {
    return external;
}

bool Command::isBackground() const {
    return background;
}

void Command::setBackground(bool background) {
    Command::background = background;
}

const string &Command::getBaseCommand() const {
    return baseCommand;
}

const vector<string> &Command::getArguments() const {
    return arguments;
}

const char *Command::getCommandLine() const {
    return commandLine;
}

Command::~Command() {
    delete commandLine;
}

bool Command::isStopped() const {
    return stopped;
}

void Command::setStopped(bool stopped) {
    Command::stopped = stopped;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {

}

void ShowPidCommand::execute() {
    int smashPid = smash.GetPid();
    cout << "smash pid is " << smashPid << endl;
}

ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}


void GetCurrDirCommand::execute() {
    string result = Utils::GetCurrentWorkingDirectoryString();
    if (result == "") {
        return;
    } else {
        cout << result << endl;
    }
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ChangeDirCommand::execute() {
    string prev_dir = smash.getLastDir();
    string curr_dir = "";
    if (arguments.size() > 1) {
        // more there one parameter passed
        cout << "smash error: cd: too many arguments" << endl;
        return;
    } else if (arguments[0] == "-") {
        // need to go back to prev dir
        if (prev_dir == "") {
            // there was no last dir
            cout << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        prev_dir = smash.getCurrDir();
        curr_dir = smash.getLastDir();
        int result = chdir(curr_dir.c_str());
        if (result == -1) {
            // chdir failed
            perror("smash error: chdir failed");
            return;
        }
    } else if (arguments.empty()) {
        // no argument what should i do ?
        return;
    } else {
        // need to change the directory to the given one
        prev_dir = Utils::GetCurrentWorkingDirectoryString();
        int result = chdir(arguments[0].c_str());
        if (result != 0) {
            // chdir failed
            perror("smash error: chdir failed");
            return;
        }
        curr_dir = arguments[0];
    }
    smash.setCurrDir(curr_dir);
    smash.setLastDir(prev_dir);
}

ChangeDirCommand::ChangeDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

ListDirectoryFilesCommand::ListDirectoryFilesCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ListDirectoryFilesCommand::execute() {
    vector<string> fileList;
    struct dirent **namelist;
    int i, n;
    n = scandir(".", &namelist, NULL, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        for (i = 0; i < n; i++) {
            fileList.push_back(namelist[i]->d_name);
        }
    }
    free(namelist);
    for (const auto &a : fileList) {
        std::cout << a << endl;
    }
}

void JobsList::printJobsList() {
    for (auto &i : jobsMap) {
        time_t now = time(nullptr);
        if (now == -1) {
            perror("smash error: time failed");
            return;
        }
        cout << "[" << i.second.getJobId() << "] " << i.second.getCommandLine() << " : "
             << i.second.getPid() << " " << difftime(now, i.second.getArriveTime()) << " secs";
        if (i.second.isStopped()) {
            cout << " (stopped)";
        }
        cout << endl;
    }
}

int JobsList::getCurrentMaxJobId() const {
    return currentMaxJobId;
}

void JobsList::setCurrentMaxJobId(int currentMaxPid) {
    JobsList::currentMaxJobId = currentMaxPid;
}

int JobsList::getCurrentMaxStoppedJobId() const {
    return currentMaxStoppedJobId;
}

void JobsList::setCurrentMaxStoppedJobId(int currentMaxStoppedPid) {
    JobsList::currentMaxStoppedJobId = currentMaxStoppedPid;
}

int JobsList::addJob(int pid, Command *cmd, bool isStopped) {
    int nJobId = getCurrentMaxJobId();
    nJobId += 1;
    JobEntry nJob(nJobId, pid, cmd);
    jobsMap.insert(std::pair<int, JobEntry>(nJobId, nJob));
    setCurrentMaxJobId(nJobId);
    return nJobId;
}

int JobsList::addJob(int jobId, int pid, Command *cmd, bool isStopped) {
    JobEntry nJob(jobId, pid, cmd);
    jobsMap.insert(std::pair<int, JobEntry>(jobId, nJob));
    return jobId;
}

void JobsList::removeJobById(int jobId) {
    JobEntry job = jobsMap.find(jobId)->second;
    job.deleteCommand();
    jobsMap.erase(jobId);
    int maxJob = getMaxKeyInMap();
    setCurrentMaxJobId(maxJob);
}

const map<int, JobsList::JobEntry> &JobsList::getJobsMap() const {
    return jobsMap;
}

const map<int, AlarmList::AlarmEntry> &AlarmList::getAlarmsMap() const {
    return alarmMap;
}

int JobsList::getMaxKeyInMap() {
    if (jobsMap.size() == 0) {
        // no jobs in the map
        return 0;
    }
    int maxJobID = 0;
    for (const auto &item : jobsMap) {
        if (item.first > maxJobID) {
            maxJobID = item.first;
        }
    }
    return maxJobID;
}

void JobsList::removeFinishedJobs() {
    int childPid;
    int status;
    childPid = waitpid(-1, &status, WNOHANG);
    while (childPid > 0) {
        int jobId = getJobIdByProcessId(childPid);
        if (jobId != 0) {
            removeJobById(jobId);
        }
        childPid = waitpid(-1, &status, WNOHANG);
    }
}

int JobsList::getJobIdByProcessId(int pid) {
    int maxJobID = 0;

    if (jobsMap.size() == 0) {
        // no jobs in the map
        return maxJobID;
    }
    for (const auto &item : jobsMap) {
        if (item.second.getPid() == pid) {
            return item.first;
        }
    }
    return maxJobID;
}

void JobsList::updateLastStoppedJobId() {
    if (smash.getJobs().getJobsMap().size() == 0) {
        currentMaxStoppedJobId = 0;
    }
    int maxJobId = 0;
    for (auto item : smash.getJobs().getJobsMap()) {
        if (item.first > maxJobId && item.second.isStopped()) {
            maxJobId = item.first;
        }
    }
    currentMaxStoppedJobId = maxJobId;
}

JobsList::JobsList() = default;

int JobsList::JobEntry::getJobId() const {
    return jobID;
}

void JobsList::JobEntry::setJobId(int jobId) {
    jobID = jobId;
}

pid_t JobsList::JobEntry::getPid() const {
    return pid;
}

void JobsList::JobEntry::setPid(pid_t pid) {
    JobEntry::pid = pid;
}

const char *JobsList::JobEntry::getCommandLine() const {
    return command->getCommandLine();
}

time_t JobsList::JobEntry::getArriveTime() const {
    return arriveTime;
}

void JobsList::JobEntry::setArriveTime(time_t arriveTime) {
    JobEntry::arriveTime = arriveTime;
}

JobsList::JobEntry::JobEntry(int jobId, int pid, Command *cmd) : jobID(jobId), pid(pid), command(cmd) {
    arriveTime = time(nullptr);
    if (arriveTime == -1) {
        // TODO: maybe fix in case of failure
        perror("smash error: time failed");
    }
}

JobsList::JobEntry::~JobEntry() {
    //delete command;
}

void JobsList::JobEntry::deleteCommand() {
    delete command;
}

bool JobsList::JobEntry::isBackground() const {
    return command->isBackground();
}

void JobsList::JobEntry::setBackground(bool set) const {
    command->setBackground(set);

}

void JobsList::JobEntry::setStopped(bool stopped) const {
    command->setStopped(stopped);
}

bool JobsList::JobEntry::isStopped() const {
    command->isStopped();
}

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {

}

void JobsCommand::execute() {
    smash.getJobsReference()->removeFinishedJobs();
    smash.getJobsReference()->printJobsList();
}

ExternalCommand::ExternalCommand(const char *cmd_line, bool isBackground) : Command(cmd_line) {
    background = isBackground;
    external = true;
}

void ExternalCommand::execute() {
    int pid = fork();
    if (pid == -1) {
        perror("smash error: fork failed");
        return;
    } else if (pid == 0) {
        setpgrp();
        //std::cout << "this is process " << getpid() << "with forked pid: " << pid << "running an external command" << endl;

        char fullArgs[COMMAND_ARGS_MAX_LENGTH] = {0};
        strcpy(fullArgs, commandLine);
        _trim(fullArgs);
        _removeBackgroundSign(fullArgs);
        char *const argsArray[] = {(char *) "/bin/bash", (char *) "-c", fullArgs, nullptr};
        int result = execv("/bin/bash", argsArray);
        if (result == -1) {
            perror("smash error: execv failed");
            return;
        }
    } else {
        // parent
        smash.getJobsReference()->removeFinishedJobs();
        int nJobId = smash.getJobsReference()->addJob(pid, this, false);
        if (!isBackground()) {
            smash.setFgPid(pid);
            waitpid(pid, nullptr, WUNTRACED);
            if (!smash.getJobs().getJobsMap().find(nJobId)->second.isStopped()) {
                // The process was not stopped while it was running, so it is safe to remove it from the jobs list
                smash.getJobsReference()->removeJobById(nJobId);
            }
            smash.getJobsReference()->updateLastStoppedJobId();
            smash.setFgPid(0);
        }
    }
}

void BackgroundCommand::execute() {
    if (arguments.size() > 1) {
        // to many args
        cout << "smash error: bg: invalid arguments" << endl;
        return;
    }
    int jobID = 0;
    if (arguments.size() == 1) {
        if (!Utils::isInteger(arguments[0])) {
            // the arguments are not numeric characters
            cout << "smash error: bg: invalid arguments" << endl;
            return;
        } else {
            jobID = std::stoi(arguments[0]);
        }
        if (smash.getJobs().getJobsMap().find(jobID) == smash.getJobs().getJobsMap().end()) {
            // there is no such job in the map
            cout << "smash error: bg: job-id " << jobID << " does not exist" << endl;
            return;
        }
        if (smash.getJobs().getJobsMap().find(jobID)->second.isBackground() &&
            !smash.getJobs().getJobsMap().find(jobID)->second.isStopped()) {
            // the job is already running at the background
            cout << "smash error: bg: job-id " << jobID << " is already running in the background" << endl;
            return;
        } else if (smash.getJobs().getJobsMap().find(jobID)->second.isBackground() &&
                   smash.getJobs().getJobsMap().find(jobID)->second.isStopped()) {
            // A specified job is stopped in the background, need to continue it's run
            string cmdLine = smash.getJobs().getJobsMap().find(jobID)->second.getCommandLine();
            int pid = smash.getJobs().getJobsMap().find(jobID)->second.getPid();
            Utils::printCommandLineFromJob(cmdLine, pid);
            if (kill(pid, SIGCONT) == -1) {
                // syscall failed
                perror("smash error: kill failed");
                return;
            }
            std::cout << "Attempting to change job " << jobID << " to status continue" << std::endl;
            smash.getJobs().getJobsMap().at(jobID).setStopped(false);
            return;
        }
    } else {
        // there is no argument, need to get the last job that stopped
        jobID = smash.getJobs().getCurrentMaxStoppedJobId();
        if (jobID == 0) {
            cout << "smash error: bg: there is no stopped jobs to resume" << endl;
            return;
        }
    }
    string cmdLine = smash.getJobs().getJobsMap().find(jobID)->second.getCommandLine();
    int pid = smash.getJobs().getJobsMap().find(jobID)->second.getPid();
    Utils::printCommandLineFromJob(cmdLine, pid);
    if (kill(pid, SIGCONT) == -1) {
        // syscall failed
        perror("smash error: kill failed");
        return;
    }
    smash.getJobs().getJobsMap().at(jobID).setStopped(false);
    smash.getJobsReference()->updateLastStoppedJobId();
}

BackgroundCommand::BackgroundCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}

void ForegroundCommand::execute() {
    if (arguments.size() > 1) {
        cout << "smash error: fg: invalid arguments" << endl;
        return;
    }
    int jobID;
    if (arguments.empty()) {
        // no job id passed
        if (smash.getJobs().getJobsMap().size() == 0) {
            // no args anf no jobs - error
            cout << "smash error: fg: jobs list is empty" << endl;
            return;
        }
        // need to take the max job id
        jobID = smash.getJobs().getCurrentMaxJobId();
    } else {
        // check if there exit such an ID at the map
        if (!Utils::isInteger(arguments[0])) {
            cout << "smash error: fg: invalid arguments" << endl;
            return;
        }
        int inputJobId = stoi(arguments[0]);
        smash.getJobsReference()->removeFinishedJobs();
        if (smash.getJobs().getJobsMap().find(inputJobId) == smash.getJobs().getJobsMap().end()) {
            // there is no such jobID
            cout << "smash error: fg: job-id " << arguments[0] << " does not exist" << endl;
            return;
        }
        jobID = inputJobId;
    }
    string cmdLine = smash.getJobs().getJobsMap().find(jobID)->second.getCommandLine();
    int jobPid = smash.getJobs().getJobsMap().find(jobID)->second.getPid();
    cout << cmdLine << " : " << jobPid << endl;
    smash.getJobs().getJobsMap().find(jobID)->second.setBackground(false);
    smash.setFgPid(jobPid);
    if (kill(jobPid, SIGCONT) == -1) {
        perror("smash error: kill failed");
        return;
    }
    smash.getJobs().getJobsMap().find(jobID)->second.setStopped(false);
    waitpid(jobPid, nullptr, WUNTRACED);
    if (!smash.getJobs().getJobsMap().find(jobID)->second.isStopped()) {
        // The process was not stopped while it was running, so it is safe to remove it from the jobs list
        smash.getJobsReference()->removeJobById(jobID);
    }
    smash.getJobsReference()->updateLastStoppedJobId();
    smash.setFgPid(0);
}

ForegroundCommand::ForegroundCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}

void KillCommand::execute() {
    if (arguments.size() != 2) {
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    // there are two arguments, need to check validity
    int signumArg = 0;
    int jobId = 0;
    if (!Utils::isInteger(arguments[0]) || !Utils::isInteger(arguments[1])) {
        // the arguments are not numeric characters
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    } else {
        signumArg = std::stoi(arguments[0]);
        jobId = std::stoi(arguments[1]);
    }
    if (jobId < 0) {
        // there is no such job id, cant send signal
        cout << "smash error: kill: job-id " << jobId << " does not exist" << endl;
        return;
    }
    if (signumArg >= 0) {
        // An invalid signal was provided
        cout << "smash error: kill: invalid arguments" << endl;
        return;
    }
    if (smash.getJobs().getJobsMap().find(jobId) == smash.getJobs().getJobsMap().end()) {
        // there is no such job id, cant send signal
        cout << "smash error: kill: job-id " << jobId << " does not exist" << endl;
        return;
    }
    int realSignNum = abs(signumArg);
    int jobPid = smash.getJobs().getJobsMap().find(jobId)->second.getPid();
    if (kill(jobPid, realSignNum) == -1) {
        perror("smash error: kill failed");
        return;
    } else if (realSignNum == 9) {
        // the process will be finished, need to remove from job list
        smash.getJobsReference()->removeJobById(jobId);
    }
    cout << "signal number " << realSignNum << " was sent to pid " << jobPid << endl;
}

KillCommand::KillCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}

QuitCommand::QuitCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}

void QuitCommand::execute() {
    // need to ignore if there were too many args
    if (!arguments.empty() && arguments[0] == "kill") {
        // we need to delete all the jobs from the job list
        smash.getJobsReference()->removeFinishedJobs();
        cout << "smash: sending SIGKILL signal to " << smash.getJobs().getJobsMap().size() << " jobs:" << endl;
        for (auto it = smash.getJobs().getJobsMap().begin(); it != smash.getJobs().getJobsMap().end(); it++) {
            int jobPid = it->second.getPid();
            string cmdLine = it->second.getCommandLine();
            cout << jobPid << ": " << cmdLine << endl;
            if (kill(jobPid, SIGKILL) == -1) {
                perror("smash error: kill failed");
            }
        }
    }
    // need to exit smash
    gotQuit = true;
}


RedirectionCommand::RedirectionCommand(const char *cmd_line, bool append, bool background) : Command(cmd_line), append(append) {
    this->background = background;
}

void RedirectionCommand::execute() {
    vector<string> input;
    if (append == false) {
        input = Utils::splitAccordingToRedirect(commandLine);
    } else {
        input = Utils::splitAccordingToAppend(commandLine);
    }
    if (input.size() != 2) {
        std::cout << "smash error: invalid argument" << std::endl;
        return;
    }
    if (input[1].empty() || input[0].empty()) {
        // dont get file name or cmd
        std::cout << "smash error: invalid argument" << std::endl;
        return;
    }
    if (background) {
        // Marking the left command as a background one
        input[0].append(" &");
        input[1] = Utils::removeBackgroundSignFromSecondCommand(input[1]);
    }
    Command *cmd = smash.CreateCommand(input[0].c_str());
    string filename = input[1];
    int result;
    int dup_res = dup(1);
    if (dup_res == -1) {
        perror("smash error: dup failed");
        delete cmd;
        return;
    }
    if (close(1) == -1) {
        perror("smash error: close failed");
        delete cmd;
        return;
    }
    if (append) {
        result = open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0666);
    } else {
        result = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    }
    if (result == -1) {
        perror("smash error: open failed");
        dup2(dup_res, 1);
        delete cmd;
        return;
    }
    smash.executeCommand(cmd->getCommandLine());
    if (close(1) == -1) {
        perror("smash error: close failed");
        dup2(dup_res, 1);
        delete cmd;
        return;
    }
    if (dup(dup_res) == -1) {
        perror("smash error: dup failed");
    }
    if (close(dup_res) == -1) {
        perror("smash error: close failed");
    }
    delete cmd;

}

PipeCommand::PipeCommand(const char *cmd_line, bool err) : Command(cmd_line), err(err) {

}

void PipeCommand::execute() {
    vector<string> input;
    if (err == false) {
        input = Utils::splitAccordingToPipe(commandLine);
    } else {
        input = Utils::splitAccordingToPipeErr(commandLine);
    }
    //vector<string> input = Utils::getBreakedCmdRedirection(commandLine, "|", "|&");
    if (input.size() != 2) {
        std::cout << "smash error: invalid arguments" << std::endl;
        return;
    }
    if (input[0].empty() || input[1].empty()) {
        std::cout << "smash error: invalid arguments" << std::endl;
        return;
    }
    int mypipe[2];
    if (pipe(mypipe) == -1) {
        perror("smash error: pipe failed");
        return;
    }
    int channel = 1;
    if (err) {
        channel = 2;
    }

    int saved = dup(channel);

    if (saved == -1) {
        perror("smash error: dup failed");
        return;
    }

    int pid1 = fork();
    if (pid1 == -1) {
        perror("smash error: fork failed");
        return;
    } else if (pid1 == 0) {
        if (dup2(mypipe[1], channel) == -1) {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(mypipe[0]) == -1) {
            perror("smash error: close failed");
            return;
        }
        if (close(mypipe[1]) == -1) {
            perror("smash error: close failed");
            return;
        }
        smash.executeCommand(input[0].c_str());
        exit(0);
    } else {
        wait(nullptr);
    }

    int pid2 = fork();
    if (pid2 == -1) {
        perror("smash error: fork failed");
        return;
    } else if (pid2 == 0) {
        if (dup2(mypipe[0], 0) == -1) {
            perror("smash error: dup2 failed");
            return;
        }
        if (close(mypipe[0]) == -1) {
            perror("smash error: close failed");
            return;
        }
        if (close(mypipe[1]) == -1) {
            perror("smash error: close failed");
            return;
        }
        smash.executeCommand(input[1].c_str());
        exit(0);
    }

    // waitpid(pid1, nullptr, WUNTRACED);
    // waitpid(pid2, nullptr, WUNTRACED);
    if (close(mypipe[0]) == -1) {
        perror("smash error: close failed");
        return;
    }
    if (close(mypipe[1]) == -1) {
        perror("smash error: close failed");
        return;
    }
    if (dup2(saved, channel) == -1) {
        perror("smash error: dup2 failed");
        return;
    }
    wait(nullptr);
}


TimeoutCommand::TimeoutCommand(const char *cmd_line, bool isBackground) : BuiltInCommand(cmd_line) {
    background = isBackground;
    external = true;
}

void TimeoutCommand::execute() {
    vector<string> brokenCommand = Utils::stringToWords(commandLine);
    if (arguments.empty() || arguments.size() < 2) {
        std::cout << "smash error: timeout: invalid arguments" << std::endl;
        return;
    }
    int duration = 0;
    if (!Utils::isInteger(arguments[0])) {
        cout << "smash error: timeout: invalid arguments" << endl;
        return;
    } else {
        duration = std::atoi(arguments[0].c_str());
        if (duration < 0) {
            cout << "smash error: timeout: invalid arguments" << endl;
            return;
        }
    }
    string realCommandString = "";
    for (int i = 1; i < arguments.size(); ++i) {
        if (realCommandString.empty()) {
            realCommandString.append(arguments[i]);
        } else {
            realCommandString.append(" " + arguments[i]);
        }
    }
    /**
     * Block to run the external command
     */
    int pid = fork();
    if (pid == -1) {
        perror("smash error: fork failed");
        return;
    } else if (pid == 0) {
        setpgrp();
        //std::cout << "this is process " << getpid() << "with forked pid: " << pid << "running an external command" << endl;

        char fullArgs[COMMAND_ARGS_MAX_LENGTH] = {0};
        strcpy(fullArgs, realCommandString.c_str());
        _trim(fullArgs);
        _removeBackgroundSign(fullArgs);
        char *const argsArray[] = {(char *) "/bin/bash", (char *) "-c", fullArgs, nullptr};
        int result = execv("/bin/bash", argsArray);
        if (result == -1) {
            perror("smash error: execv failed");
            return;
        }
    } else {
        // parent (and also the real timeout command)
        smash.getJobsReference()->removeFinishedJobs();
        int nJobId = smash.getJobsReference()->addJob(pid, this, false);
        int len = strlen(commandLine) + 1;
        char *cmd = (char *) malloc(len);
        strcpy(cmd, commandLine);
        int alarmId = smash.getAlarmsReference()->addAlarm(nJobId, pid, duration, cmd);
        time_t now = time(nullptr);
        smash.getAlarmsReference()->scheduleNextAlarm(now);
        if (!isBackground()) {
            smash.setFgPid(pid);
            waitpid(pid, nullptr, WUNTRACED);
            if (!smash.getJobs().getJobsMap().find(nJobId)->second.isStopped()) {
                // The process was not stopped while it was running, so it is safe to remove it from the jobs list
                smash.getJobsReference()->removeJobById(nJobId);
                smash.getAlarmsReference()->removeAlarmById(alarmId);
            }
            smash.getJobsReference()->updateLastStoppedJobId();
            smash.setFgPid(0);
        }
    }
}

AlarmList::AlarmEntry::AlarmEntry(int id, int jobId, int realPid, int alarmDuration, char *originalCommand) : id(id), jobId(jobId), realPid(realPid),
                                                                                                              originalAlarmDuration(alarmDuration),
                                                                                                              originalCommand(originalCommand) {
    arriveTime = time(nullptr);
    if (arriveTime == -1) {
        // TODO: maybe fix in case of failure
        perror("smash error: time failed");
    }
}

time_t AlarmList::AlarmEntry::getArriveTime() const {
    return arriveTime;
}

int AlarmList::AlarmEntry::getOriginalDuration() const {
    return originalAlarmDuration;
}

int AlarmList::AlarmEntry::getJobId() const {
    return jobId;
}

int AlarmList::AlarmEntry::getRealPid() const {
    return realPid;
}

char *AlarmList::AlarmEntry::getOriginalCommand() const {
    return originalCommand;
}

int AlarmList::getMaxId() {
    return maxAlarmId;
}

int AlarmList::addAlarm(int jobId, int realPid, int alarmDuration, char *originalCommand) {
    int nAlarmId = getMaxId();
    nAlarmId += 1;
    AlarmEntry nAlarm(nAlarmId, jobId, realPid, alarmDuration, originalCommand);
    alarmMap.insert(std::pair<int, AlarmEntry>(nAlarmId, nAlarm));
    setMaxAlarmId(nAlarmId);
    return nAlarmId;
}

int AlarmList::getAlarmIdOfExpiredAlarm(time_t now) {
    for (auto &it : alarmMap) {
        int timeLeft = it.second.getOriginalDuration() - difftime(now, it.second.getArriveTime());
        if (timeLeft <= 0) {
            // This command has expired, and needs to be killed
            return it.first;
        }
    }
    return -1;
}

int AlarmList::getJobIdOfExpiredAlarm(time_t now) {
    for (auto &it : alarmMap) {
        int timeLeft = difftime(now, it.second.getArriveTime()) - it.second.getOriginalDuration();
        if (timeLeft <= 0) {
            // This command has expired, and needs to be killed
            return it.second.getJobId();
        }
    }
    return -1;
}

void AlarmList::removeAlarmById(int alarmId) {
    AlarmEntry alarm = alarmMap.find(alarmId)->second;
    alarmMap.erase(alarmId);
    int maxJob = getMaxKeyInMap();
    setMaxAlarmId(maxJob);
}

AlarmList::AlarmList() = default;

int AlarmList::getMaxKeyInMap() {
    if (alarmMap.size() == 0) {
        // no jobs in the map
        return 0;
    }
    int maxJobID = 0;
    for (const auto &item : alarmMap) {
        if (item.first > maxJobID) {
            maxJobID = item.first;
        }
    }
    return maxJobID;
}

void AlarmList::setMaxAlarmId(int maxAlarmId) {
    AlarmList::maxAlarmId = maxAlarmId;
}

void AlarmList::updateMaxAlarmId() {
    if (alarmMap.size() == 0) {
        maxAlarmId = 0;
    }
    int maxId = 0;
    for (auto item : alarmMap) {
        if (item.first > maxId) {
            maxId = item.first;
        }
    }
    maxAlarmId = maxId;
}

void AlarmList::scheduleNextAlarm(time_t now) {
    int upcomingAlarm = -1;
    if (alarmMap.empty()) {
        return;
    }
    for (auto &it : alarmMap) {
        int timePassedSinceArrival = difftime(now, it.second.getArriveTime());
        int remaining = it.second.getOriginalDuration() - timePassedSinceArrival;
        if (remaining < upcomingAlarm || upcomingAlarm == -1) {
            upcomingAlarm = remaining;
        }
    }
    alarm(upcomingAlarm);
}


CopyCommand::CopyCommand(const char *cmd_line, bool background) : BuiltInCommand(cmd_line) {
    setBackground(background);
    external = true;
}

void CopyCommand::execute() {
    // check valid
    if (arguments.size() < 2) {
        std::cout << "smash error: invalid arguments" << endl;
        return;
    }
    int pid = fork();
    if (pid == -1) {
        perror("smash error : fork failed");
        return;
    }

    if (pid == 0) {
        // child
        bool copy = false;
        int fd = open(arguments[0].c_str(), O_RDONLY);
        if (fd == -1) {
            perror("smash error: open failed");
            return;
        }
        ifstream in_file(arguments[0].c_str(), ios::binary);
        in_file.seekg(0, ios::end);
        int fileSize = in_file.tellg();

        char *temp_src = realpath(arguments[0].c_str(), nullptr);
        if (temp_src == nullptr) {
            perror("smash error: realpath failed");
            return;
        }

        char *temp_dst = realpath(arguments[1].c_str(), nullptr);
        if (temp_dst != nullptr && strcmp(temp_dst, temp_src) == 0) {
            // same path
            free(temp_dst);
            free(temp_src);
            if (!this->isBackground()) {
                std::cout << "smash: " << arguments[0] << " was copied to " << arguments[1] << endl;
            }
            return;
        } else {
            free(temp_src);
        }
        if (temp_dst != nullptr) {
            free(temp_dst);
        }

        int destFd = open(arguments[1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (destFd == -1) {
            perror("smash error: open failed");
            if (close(fd) == -1) {
                perror("smash error: close failed");
            }
            return;
        }
        while (fileSize != 0) {
            void *buff = malloc(1);
            int readNum = read(fd, buff, 1);
            if (readNum != 1) {
                perror("smash error: read failed");
                free(buff);
                return;
            }
            int writeNum = write(destFd, buff, 1);
            if (writeNum != 1) {
                perror("smash error: write failed");
                free(buff);
                return;
            }
            // if got here, succeed read and write one byte
            --fileSize;
            free(buff);
        }
        if (!this->isBackground()) {
            std::cout << "smash: " << arguments[0] << " was copied to " << arguments[1] << endl;
        }

//        while (!copy) {
//            int readNum = read(fd, buff, fileSize);
//            if (readNum <= 0) {
//                if (readNum != 0) {
//                    perror("smash error: read failed");
//                    // return;
//                } else {
//                    if(!this->isBackground()) {
//                        std::cout << "smash: " << arguments[0] << " was copied to " << arguments[1] << endl;
//                    }
//                }
//                copy = true;
//            } else {
//                int writeNum = write(destFd, buff, readNum);
//                if (writeNum != readNum) {
//                    perror("smash error: write failed");
//                    copy = true;
//                }
//            }
//        }
//        free(buff);
        fd = close(fd);
        destFd = close(destFd);
        if (fd == -1 || destFd == -1) {
            perror("smash error: close failed");
        }
        return;
//    } else {
//        // parent
//        if (!this->isBackground()) {
//            smash.setFgPid(pid);
//            waitpid(pid, nullptr, WUNTRACED);
//            smash.setFgPid(0);
//        } else {
//            smash.getJobsReference()->addJob(pid, this, false);
//            return;
//        }
//    }
    } else {
        // parent
        smash.getJobsReference()->removeFinishedJobs();
        int nJobId = smash.getJobsReference()->addJob(pid, this, false);
        if (!isBackground()) {
            smash.setFgPid(pid);
            waitpid(pid, nullptr, WUNTRACED);
            if (!smash.getJobs().getJobsMap().find(nJobId)->second.isStopped()) {
                // The process was not stopped while it was running, so it is safe to remove it from the jobs list
                smash.getJobsReference()->removeJobById(nJobId);
            }
            smash.getJobsReference()->updateLastStoppedJobId();
            smash.setFgPid(0);
        }


    }
}
