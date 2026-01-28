
#pragma once

// TODO: Move these head importation into cpp file
#include <iostream>
#include <sstream>
#include <iomanip>
#include <sys/time.h>

class LogMessage
{
public:
  LogMessage(const char *file, int line, const char *func, int level);
  ~LogMessage();
  std::ostream& stream();

private:
  void Init(const char *file, int line, const char *func);

private:
  int level_;
  std::ostringstream stream_;

  const char *file_;
  const int line_;
  const char *func_;
};

#define LOG(condition) LogMessage(__FILE__, __LINE__, __FUNCTION__, condition).stream()

