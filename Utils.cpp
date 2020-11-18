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
