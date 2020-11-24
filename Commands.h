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
    char *commandLine;
    string baseCommand = "BASE_PLACEHOLDER";
    vector<string> arguments;
    bool stopped = false;
    bool external = false;
    bool background = false;

public:
    Command(const char *cmd_line);

    virtual ~Command();

    virtual void execute() = 0;

    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
    bool isExternal() const;

    bool isBackground() const;

    void setBackground(bool background);

    bool isStopped() const;

    void setStopped(bool stopped);

    const char *getCommandLine() const;

    const string &getBaseCommand() const;

    const vector<string> &getArguments() const;

};

class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line);

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line, bool isBackground);

    virtual ~ExternalCommand() {}

    void execute() override;
};

class PipeCommand : public Command {
    bool err = false;
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line, bool err);

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
    bool append;

public:
    explicit RedirectionCommand(const char *cmd_line, bool append);

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

class MorCommand : public BuiltInCommand {
public:
    explicit MorCommand(const char *cmd_line);

    virtual ~MorCommand() {}

    void execute() override;
};

class JobsList;

class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    QuitCommand(const char *cmdLine);

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

class AlarmList {

};

class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    private:
        int jobID = 0;
        int pid;
        Command *command;
        time_t arriveTime;
        bool stopped = false;
    public:
        JobEntry(int jobId, int pid, Command *cmd);

        int getJobId() const;

        void setJobId(int jobId);

        int getPid() const;

        void setPid(int pid);

        const char *getCommandLine() const;

        time_t getArriveTime() const;

        void setArriveTime(time_t arriveTime);

        bool isStopped() const;

        void setStopped(bool stopped) const;

        bool isBackground() const;

        void setBackground(bool set) const;

        void deleteCommand();

        ~JobEntry();
    };

private:
    // TODO: Add your data members
    int currentMaxJobId = 0;
    int currentMaxStoppedJobId = 0;
    std::map<int, JobEntry> jobsMap;
public:
    const std::map<int, JobEntry> &getJobsMap() const;

    JobsList();

    ~JobsList() {};

    void addJob(Command *cmd, bool isStopped = false);

    int addJob(int pid, int jobId, Command *cmd, bool isStopped = false);

    int addJob(int jobId, Command *cmd, bool isStopped = false);

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    void updateLastStoppedJobId();

    // TODO: Add extra methods or modify exisitng ones as needed
    int getCurrentMaxJobId() const;

    void setCurrentMaxJobId(int currentMaxPid);

    int getCurrentMaxStoppedJobId() const;

    void setCurrentMaxStoppedJobId(int currentMaxStoppedPid);

    int getMaxKeyInMap();

    int getJobIdByProcessId(int pid);

};

class JobsCommand : public BuiltInCommand {
    JobsList *jobs;
public:
    //JobsCommand(const char *cmdLine, const char *cmd_line, JobsList *jobs);
    JobsCommand(const char *cmd_line, JobsList *jobs);

    virtual ~JobsCommand() {}

    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmdLine);

    virtual ~KillCommand() {}

    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmdLine);

    virtual ~ForegroundCommand() {}

    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char *cmdLine);

    virtual ~BackgroundCommand() {}

    void execute() override;
};

class ListDirectoryFilesCommand : public BuiltInCommand {
public:
    ListDirectoryFilesCommand(const char *cmd_line);

    virtual ~ListDirectoryFilesCommand() {}

    void execute() override;
};

class TimeoutCommand : public BuiltInCommand {
public:
    TimeoutCommand(const char *cmd_line);

    virtual ~TimeoutCommand() {}

    void execute() override;
};

// TODO: add more classes if needed 
// maybe ls, timeout ?

class SmallShell {
private:
    // TODO: Add your data members
    SmallShell();

    JobsList jobs;
    int pid = 0;
    string prompt = "smash> ";
    string last_dir = "";
    string curr_dir = "";
    int fgPid = 0;
public:
    const string &getLastDir() const;

    const string &getCurrDir() const;

    void setCurrDir(string currDir);

    void setLastDir(string lastDir);

    Command *CreateCommand(const char *cmd_line);

    void setFgPid(int fgPid);

    int getFgPid() const;

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

    void setJobs(JobsList &jobs);

    string GetPrompt();

    void SetPrompt(string prompt);

};

#endif //SMASH_COMMAND_H_
