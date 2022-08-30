#include "processor.h"

#include "linux_parser.h"

float Processor::Utilization() {
  return LinuxParser::ActiveJiffies() / float(LinuxParser::Jiffies());
}
