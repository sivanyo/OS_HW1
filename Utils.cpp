//
// Created by 1912m on 18/11/2020.
//

#include <iostream>
#include <unistd.h>
#include "Utils.h"

using std::cout;
using std::endl;

vector<string> Utils::stringToWords(const string& s) {
    vector<string> result;
    string word = "";
    for (auto x: s) {
        if (x == ' ' && word != "") {
            result.push_back(word);
            word = "";
        } else {
            word = word + x;
        }
    }
    if (word != "") {
        result.push_back(word);
    }
    return result;
}

bool Utils::isBackgroundCommand(const string &s) {
    vector<string> res = stringToWords(s);
    if(res[res.size()-1] == "&"){
        return true;
    }
    return false;
}

bool Utils::isRedirectionCommand(const string &s) {
    // TODO: need to fix other redirect flag checkers in a similar manner (sivan)
    vector<string> res = stringToWords(s);
    for(int i = 0 ; i < res.size() ; i++){
        if(res[i].find(">") == 0){
            return true;
        }
    }
    return false;
}

bool Utils::isRedirectionCommandWithAppend(const string &s) {
    vector<string> res = stringToWords(s);
    for(int i = 0 ; i < res.size() ; i++){
        if(res[i] == ">>"){
            return true;
        }
    }
    return false;
}

bool Utils::isPipeout(const string &s) {
    vector<string> res = stringToWords(s);
    for(int i = 0 ; i < res.size() ; i++){
        if(res[i] == "|"){
            return true;
        }
    }
    return false;
}

bool Utils::isPipeErr(const string &s) {
    vector<string> res = stringToWords(s);
    for(int i = 0 ; i < res.size() ; i++){
        if(res[i] == "|&"){
            return true;
        }
    }
    return false;
}

string Utils::GetCurrentWorkingDirectoryString() {
    char *currDirCommand = get_current_dir_name();
    if (currDirCommand == NULL) {
        perror("ERROR : get_current_dir_name failed");
        return "";
    }
    string result = currDirCommand;
    free(currDirCommand);
    return result;
}

bool Utils::isInteger(const std::string & s)
{
    if(s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;

    char * p;
    strtol(s.c_str(), &p, 10);

    return (*p == 0);
}

void Utils::printCommandLineFromJob(string cmdline, int pid) {
    std::cout << cmdline << " : " << pid << std::endl;
}

vector<string> Utils::getBreakedCmdRedirection(const string& s, const string& s1, const string& s2) {
    // TODO: this should be able to handle cases where special symbols (>,>>,|,&) are not spaced from normal arguments
    vector<string> sentence = stringToWords(s);
    vector<string> result;
    bool after = false;
    string cmd1="";
    string cmd2="";
    for(int i = 0 ; i < sentence.size() ; i++){
        if(sentence[i] == s1 || sentence[i] == s2){
            after= true;
            continue;
        }
        if(!after){
            if (cmd1.empty()) {
                cmd1.append(sentence[i]);
            } else {
                cmd1.append(" " + sentence[i]);
            }
        }
        else{
            if (cmd2.empty()) {
                cmd2.append(sentence[i]);
            } else {
                cmd2.append(" " + sentence[i]);
            }

        }
    }
    result.push_back(cmd1);
    result.push_back(cmd2);
    return result;
}
