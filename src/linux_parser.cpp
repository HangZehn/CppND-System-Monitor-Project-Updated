#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.emplace_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  string name;
  float total_mem;
  float free_mem;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "MemTotal:") {
        linestream >> total_mem;
      }
      if (key == "MemFree:") {
        linestream >> free_mem;
      }
    }
  }
  return (total_mem - free_mem) / total_mem;
}

long LinuxParser::UpTime() {
  string line;
  string uptime_str;
  long up_time;
  std::ifstream filestream(kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    linestream >> uptime_str;
  }
  up_time = std::stol(uptime_str);
  return up_time;
}

long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies() + LinuxParser::IdleJiffies();
}

long LinuxParser::ActiveJiffies(int pid) {
  string line;
  string key;
  string value;
  long totaltime;
  vector<string> proc_jiffies;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> value) {
        proc_jiffies.emplace_back(value);
      }
      totaltime = std::stol(proc_jiffies[13]) + std::stol(proc_jiffies[14]) +
                  std::stol(proc_jiffies[15]) + std::stol(proc_jiffies[16]);
    }
  }
  return totaltime;
}

long LinuxParser::ActiveJiffies() {
  vector<string> jiffies = CpuUtilization();
  return std::stol(jiffies[CPUStates::kUser_]) +
         std::stol(jiffies[CPUStates::kNice_]) +
         std::stol(jiffies[CPUStates::kSystem_]) +
         std::stol(jiffies[CPUStates::kIRQ_]) +
         std::stol(jiffies[CPUStates::kSoftIRQ_]) +
         std::stol(jiffies[CPUStates::kSteal_]) +
         std::stol(jiffies[CPUStates::kGuest_]) +
         std::stol(jiffies[CPUStates::kGuestNice_]);
}

long LinuxParser::IdleJiffies() {
  vector<string> jiffies = CpuUtilization();
  return std::stol(jiffies[CPUStates::kIdle_]) +
         std::stol(jiffies[CPUStates::kIOwait_]);
}

vector<string> LinuxParser::CpuUtilization() {
  string line;
  string key;
  string value;
  vector<string> jiffies;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      while (linestream >> value) {
        jiffies.emplace_back(value);
      }
    }
  }
  return jiffies;
}

int LinuxParser::TotalProcesses() {
  string line;
  string key;
  int total_process;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "processes") {
        linestream >> total_process;
      }
    }
  }
  return total_process;
}

int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int running_process;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "procs_running") {
        linestream >> running_process;
      }
    }
  }
  return running_process;
}

string LinuxParser::Command(int pid) {
  string line;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
  }
  return line;
}

string LinuxParser::Ram(int pid) {
  string line;
  string key;
  string memory;
  int memory_int;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key ==
          "VmRSS:") {  // recommended to use VmRSS instead of VmSize, by review:
                       // https://review.udacity.com/#!/reviews/3750075
        linestream >> memory_int;
        memory = std::to_string(memory_int / 1000);
      }
    }
  }
  return memory;
}

string LinuxParser::Uid(int pid) {
  string line;
  string key;
  string uid;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> uid;
      }
    }
  }
  return uid;
}

string LinuxParser::User(int pid) {
  string line;
  string key;
  string name;
  string uid;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> key >> name >> uid;
      if (uid == Uid(pid)) {
        return key;
      }
    }
  }
  return std::string();
}

long LinuxParser::UpTime(int pid) {
  string line;
  string key;
  vector<string> values;
  std::ifstream filestream(kProcDirectory + std::to_string(pid) +
                           kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key) {
        values.emplace_back(key);
      }
    }
  }
  // return std::stol(values[21]);
  int upTimePid = UpTime() - stol(values[21]) / sysconf(_SC_CLK_TCK);
  return upTimePid;
}
