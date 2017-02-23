#ifndef __JOB_QUEUE_H__
#define __JOB_QUEUE_H__

#include "job_struct.h"
#include "deque.h"

IMPLEMENT_DEQUE_STRUCT (job_queue, job_struct);

PROTOTYPE_DEQUE (job_queue, job_struct);

#endif
