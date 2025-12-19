#ifndef _WHITE_OS_TASK_H
#define _WHITE_OS_TASK_H

#include <stdint.h>
#include <stddef.h>

#include "memory.h"

typedef int32_t pid_t;

enum class TaskState
{
  NEW,
  READY,
  RUNNING,
  BLOCKED,
  EXITED
};

class Task
{
public:
  Task *prev;
  Task *next;
  pid_t pid;
  TaskState state;
  PageTable *pageTable;
  uint64_t kernelStackStart;
  uint64_t kernelStackTop;
  uint64_t userStackStart;
  uint64_t userStackTop;
};

#endif