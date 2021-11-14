#ifndef __UTILS__
#define __UTILS__


#include <string>


std::string intToString(uint32_t number) {
  char c_str[30];
  memset(c_str, 0, 30);
  itoa(number, c_str, 10);
  std::string tempStr(c_str);
  return tempStr;
}

uint32_t stringToInt(std::string str) {
  return atoi(str.c_str());
}

#endif
