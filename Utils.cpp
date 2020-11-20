//
// Created by 1912m on 18/11/2020.
//

#include <unistd.h>
#include "Utils.h"

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
