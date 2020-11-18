//
// Created by 1912m on 18/11/2020.
//

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
