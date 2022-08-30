#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) {
  pid_ = pid;
  // long process_jiffies = LinuxParser::ActiveJiffies(Pid());
  // long total_jiffies = LinuxParser::Jiffies();
  // cpu_util_ = float(process_jiffies) / total_jiffies;
  cpu_util_ = LinuxParser::ActiveJiffies(Pid()) / float(LinuxParser::Jiffies());
}

int Process::Pid() { return pid_; }

float Process::CpuUtilization() const { return cpu_util_; }

string Process::Command() { return LinuxParser::Command(Pid()); }

string Process::Ram() { return LinuxParser::Ram(Pid()); }

string Process::User() { return LinuxParser::User(Pid()); }

long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

bool Process::operator<(Process const& a) const {
  return a.CpuUtilization() < CpuUtilization();
}
