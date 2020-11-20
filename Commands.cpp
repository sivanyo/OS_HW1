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
#include "Commands.h"
#include "Utils.h"

extern SmallShell &smash;

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
    string command = string(cmd_line);
    bool background = _isBackgroundComamnd(cmd_line);
    bool redirectOutput = Utils::isRedirectionCommand(command);
    bool redirectAppend = Utils::isRedirectionCommandWithAppend(command);
    bool pipe = Utils::isPipe(command);
    bool pipeRedirect = Utils::isPipeAndRedirect(command);

    // TODO: add command to detect pipeline
    // TODO: add command to detect & sign
    if (command.find("chprompt") == 0) {
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

    } else if (command.find("fg") == 0) {
        return new ForegroundCommand(cmd_line);
    } else if (command.find("bg") == 0) {

    } else if (command.find("quit") == 0) {

        /*
         * This part should include certain boolean flags for special commands (maybe should be above internal commands ifs)
         */
    } else {
        // External command
        return new ExternalCommand(cmd_line, background);
    }

//    } else if (command.find("showpid") == 0) {
//        return
//    }
    // For example:
/*
  string cmd_s = string(cmd_line);
  if (cmd_s.find("pwd") == 0) {
    return new GetCurrDirCommand(cmd_line);
  }
  else if ...
  .....
  else {
    return new ExternalCommand(cmd_line);
  }
  */
    return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command *cmd = CreateCommand(cmd_line);
    if (cmd->isExternal()) {
        cmd->execute();
    } else {
        cmd->execute();
        delete cmd;
    }

    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)
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

//int SmallShell::getJobsListSize() {
//    return smash.jobs.;
//}

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
        if (result != 0) {
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
            if (strcmp(namelist[i]->d_name, ".") != 0 && strcmp(namelist[i]->d_name, "..") != 0) {
                fileList.push_back(namelist[i]->d_name);
            }
        }
    }
    free(namelist);

    sort(fileList.begin(), fileList.end(), [](const auto &lhs, const auto &rhs) {
        const auto result = mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(),
                                     [](const unsigned char lhs, const unsigned char rhs) { return tolower(lhs) == tolower(rhs); });

        return result.second != rhs.cend() && (result.first == lhs.cend() || tolower(*result.first) < tolower(*result.second));
    });
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
             << i.second.getPid() << " " << difftime(now, i.second.getArriveTime()) << " secs ";
        if (i.second.isStopped()) {
            cout << "(stopped)";
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
}

const map<int, JobsList::JobEntry> &JobsList::getJobsMap() const {
    return jobsMap;
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

bool JobsList::JobEntry::isStopped() const {
    return stopped;
}

void JobsList::JobEntry::setStopped(bool stopped) {
    JobEntry::stopped = stopped;
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

JobsCommand::JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line), jobs(jobs) {

}

void JobsCommand::execute() {
    smash.getJobsReference()->printJobsList();
}

ExternalCommand::ExternalCommand(const char *cmd_line, bool isBackground) : Command(cmd_line) {
    background = isBackground;
    external = true;
}

void ExternalCommand::execute() {
//    char fullArgs[COMMAND_ARGS_MAX_LENGTH] = {0};
//    strcpy(fullArgs, commandLine);
//    _trim(fullArgs);
//    _removeBackgroundSign(fullArgs);
    int pid = fork();
    if (pid == -1) {
        perror("smash error: fork failed");
        return;
    } else if (pid == 0) {
        setpgrp();
        std::cout << "this is process " << getpid() << "with forked pid: " << pid << "running an external command" << endl;

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
        // TODO: remove this job after waiting is done
        int nJobId = smash.getJobsReference()->addJob(pid, this, false);
        if (!isBackground()) {
            std::cout << "waiting for job: " << nJobId << " with pid: " << pid << std::endl;
            waitpid(pid, nullptr, 0);
        } else {
            std::cout << "running job: " << nJobId << " in the background with pid: " << pid << std::endl;
        }
    }


}

void BackgroundCommand::execute() {

}

void ForegroundCommand::execute() {
    if (arguments.size() > 1) {
        cout << "smash error: fg: invalid arguments" << endl;
        return;
    }
    int jobID;
    if (arguments.size() == 0) {
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
        int inputJobId = stoi(arguments[0]);
        if (smash.getJobs().getJobsMap().find(inputJobId) == smash.getJobs().getJobsMap().end()) {
            // there is no such jobID :(
            cout << "smash error: fg: job-id " << arguments[0] << " does not exist" << endl;
            return;
        }
        jobID = inputJobId;
    }
    string cmdLine = smash.getJobs().getJobsMap().find(jobID)->second.getCommandLine();
    int jobPid = smash.getJobs().getJobsMap().find(jobID)->second.getPid();
    cout << cmdLine << " : " << jobPid << endl;
    smash.getJobs().getJobsMap().find(jobID)->second.setBackground(false);
    if (killpg(jobPid, SIGCONT) == -1) {
        perror("smash error: kill failed");
        return;
    }
    waitpid(jobPid, nullptr, 0);
}

ForegroundCommand::ForegroundCommand(const char *cmdLine) : BuiltInCommand(cmdLine) {

}
