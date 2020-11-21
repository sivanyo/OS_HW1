//
// Created by 1912m on 18/11/2020.
//

#ifndef HW1_UTILS_H
#define HW1_UTILS_H

#include <string>
#include <vector>

using std::vector;
using std::string;

class Utils {
public:
    static vector<string> stringToWords(const string& s);
    static bool isBackgroundCommand(const string& s);
    static bool isRedirectionCommand(const string& s);
    static bool isRedirectionCommandWithAppend(const string& s);
    static bool isPipe(const string& s);
    static bool isPipeAndRedirect(const string& s);

    static string GetCurrentWorkingDirectoryString();

    static bool isInteger(const std::string & s);
};


#endif //HW1_UTILS_H
