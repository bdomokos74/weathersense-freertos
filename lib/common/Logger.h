// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: MIT

#ifndef _SLLOGGER_H
#define _SLLOGGER_H


class Logger
{
private:
  void writeTime(char *buf);
public:
  Logger();
  void info(const char *message);
  void info(const char *message, int i);
  void error(const char *message);
  void error(const char *message, int i);
  void printBuf(const char *header, const char *data, int len);
  void println(const char *str1, const char *str2);
  void println(const char *str1, int i);
  void println(const char *str);
};

extern Logger logger;

#endif // _SLLOGGER_H
