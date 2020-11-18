#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <iostream>
#include <algorithm>
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

SmallShell::SmallShell() {
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
    if (command.find("chprompt") == 0) {
        return new ChangePromptCommand(cmd_line);
    } else if (command.find("ls") == 0) {
        return new ListDirectoryFilesCommand(cmd_line);
    } else if (command.find("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (command.find("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
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
    cmd->execute();
    delete cmd;
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

vector<string> stringToWords (string s){
    vector<string> result;
    string word="";
    for (auto x: s){
        if(x ==' ' && word!=""){
            result.push_back(word);
            word="";
        }
        else{
            word=word+x;
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
    commandLine = string(cmd_line);
    vector<string> split = Utils::stringToWords(commandLine);
    baseCommand = split[0];
    for (int i = 1; i < split.size(); ++i) {
        arguments.push_back(split[i]);
    }
    return result;
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
    char *currDirCommand = get_current_dir_name();
    if (currDirCommand == NULL) {
        perror("ERROR : get_current_dir_name failed");
        return;
    }
    cout << currDirCommand << endl;
    delete currDirCommand;
}

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ChangeDirCommand::execute() {
    string prev_dir = smash.getLastDir();
    string curr_dir = "";
    if (arguments.size() > 1){
        // more there one parameter passed
        cout << "smash error: cd: too many arguments" << endl;
    }
    else if(arguments[0] == "-") {
        // need to go back to prev dir
        if (prev_dir == "") {
            // there was no last dir
            cout << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        curr_dir = smash.getLastDir();
    }
    else if (arguments.empty()){
        // no argument what should i do ?
        return;
    }
    else{
        // need to change the directory to the given one
        int result = chdir(arguments[0]);
        if (result != 0){
            // chdir failed
            perror("smash error: chdir failed");
            return;
        }
        curr_dir = arguments[0];
    }
    smash.setCurrDir(curr_dir);
    smash.setLastDir(prev_dir);
}

ListDirectoryFilesCommand::ListDirectoryFilesCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}

void ListDirectoryFilesCommand::execute() {
    struct dirent **namelist;
    int i,n;
    n = scandir(".", &namelist, 0, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        for (i = 0; i < n; i++) {
            printf("%s\n", namelist[i]->d_name);
            free(namelist[i]);
        }
    }
    free(namelist);
//    char *currDirCommand = get_current_dir_name();
//    vector<string> fileList;
//    struct dirent *entry;
//    DIR *dir = opendir(currDirCommand);
//
//    if (dir == nullptr) {
//        return;
//    }
//    while ((entry = readdir(dir)) != nullptr) {
//        int firstComp = strcmp(entry->d_name, ".");
//        int secondComp = strcmp(entry->d_name, "..");
//        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
//            fileList.push_back(entry->d_name);
//        }
//    }
//    std::sort(fileList.begin(), fileList.end());
//
//    for (const auto& a : fileList) {
//        std::cout << a << endl;
//    }
//
//    closedir(dir);
//    delete currDirCommand;
}
