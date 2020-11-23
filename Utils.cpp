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
    vector<string> res = stringToWords(s);
    for(int i = 0 ; i < res.size() ; i++){
        if(res[i] == ">"){
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

bool Utils::isPipe(const string &s) {
    vector<string> res = stringToWords(s);
    for(int i = 0 ; i < res.size() ; i++){
        if(res[i] == "|"){
            return true;
        }
    }
    return false;
}

bool Utils::isPipeAndRedirect(const string &s) {
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
    vector<string> sentence = stringToWords(s);
    vector<string> result;
    bool after = false;
    string cmd="";
    string filename="";
    for(int i = 0 ; i < sentence.size() ; i++){
        if(sentence[i] == s1 || sentence[i] == s2){
            after= true;
            continue;
        }
        if(!after){
            cmd.append(" "+ sentence[i]);
        }
        else{
            filename.append(sentence[i]);
        }
    }
    result.push_back(cmd);
    result.push_back(filename);
    return result;
}
