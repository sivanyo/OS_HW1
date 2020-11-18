#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <string>
#include <vector>
#include <time.h>
#include <map>

using std::string;
using std::vector;
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)

class Command {
protected:
    int pid = -1;
    int jobId = -1;
    string commandLine = "PLACEHOLDER";
    string baseCommand = "BASE_PLACEHOLDER";
    vector<string> arguments;
    bool stopped = false;
    time_t startTime = time(nullptr);

public:
    Command(const char *cmd_line);

    virtual ~Command() {};

    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line);

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
    bool background = false;
public:
    ExternalCommand(const char *cmd_line, bool background);

    virtual ~ExternalCommand() {}

    bool isBackground() const;

    void setBackground(bool background);

    void execute() override;
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);

    virtual ~RedirectionCommand() {}

    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};

class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members
public:
    ChangeDirCommand(const char *cmd_line);

    virtual ~ChangeDirCommand() {}

    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};

class ChangePromptCommand : public BuiltInCommand {
public:
    explicit ChangePromptCommand(const char *cmd_line);

    virtual ~ChangePromptCommand() {}

    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
public:
    explicit ShowPidCommand(const char *cmd_line);

    virtual ~ShowPidCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
    QuitCommand(const char *cmd_line, JobsList *jobs);

    virtual ~QuitCommand() {}

    void execute() override;
};

class CommandsHistory {
protected:
    class CommandHistoryEntry {
        // TODO: Add your data members
    };
    // TODO: Add your data members
public:
    CommandsHistory();

    ~CommandsHistory() {}

    void addRecord(const char *cmd_line);

    void printHistory();
};

class HistoryCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    HistoryCommand(const char *cmd_line, CommandsHistory *history);

    virtual ~HistoryCommand() {}

    void execute() override;
};

class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    private:
        int jobID;
        pid_t pid;
        string commandLine;
        time_t arriveTime;
        bool stopped = false;
    public:
        int getJobId() const;

        void setJobId(int jobId);

        pid_t getPid() const;

        void setPid(pid_t pid);

        const string &getCommandLine() const;

        void setCommandLine(const string &commandLine);

        time_t getArriveTime() const;

        void setArriveTime(time_t arriveTime);

        bool isStopped() const;

        void setStopped(bool stopped);
    };

private:
    // TODO: Add your data members
    int currentMaxPid = 0;
    int currentMaxStoppedPid = 0;
    std::map<int, JobEntry> jobsMap;
public:
    JobsList();

    ~JobsList() {};

    void addJob(Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
    int getCurrentMaxPid() const;

    void setCurrentMaxPid(int currentMaxPid);

    int getCurrentMaxStoppedPid() const;

    void setCurrentMaxStoppedPid(int currentMaxStoppedPid);
};

class JobsCommand : public BuiltInCommand {
    JobsList* jobs;
public:
    //JobsCommand(const char *cmdLine, const char *cmd_line, JobsList *jobs);
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char *cmd_line, JobsList *jobs);

    virtual ~BackgroundCommand() {}

    void execute() override;
};

class ListDirectoryFilesCommand : public BuiltInCommand {
public:
    ListDirectoryFilesCommand(const char *cmd_line);

    virtual ~ListDirectoryFilesCommand() {}

    void execute() override;
};

// TODO: add more classes if needed 
// maybe ls, timeout ?

class SmallShell {
private:
    // TODO: Add your data members
    string prompt = "smash> ";
public:
    const string &getLastDir() const;

private:
    string last_dir = "";
public:
    void setCurrDir(string currDir);

private:
    string curr_dir = "";
public:
    void setLastDir(string lastDir);

public:
    const string &getCurrDir() const;

private:
    SmallShell();
    JobsList jobs;
    int pid = 0;

public:
    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed

    int GetPid();

    const JobsList &getJobs() const;
    JobsList *getJobsReference();

    void setJobs(const JobsList &jobs);

    string GetPrompt();

    void SetPrompt(string prompt);

};

#endif //SMASH_COMMAND_H_
