#pragma once

#include <string>

enum class severity;

enum class logopt : unsigned;

inline logopt operator|(logopt a, logopt b);
inline logopt& operator|=(logopt& a, logopt b);

void Log(severity level, const std::string& message, logopt opts = logopt::NONE, std::string cmd = "");
