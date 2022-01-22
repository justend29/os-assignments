/*
 * clang-format off
 * File Name: a1.c
 * Author: Justen G. Di Ruscio - 3624673
 * Institution: University of New Brunswick
 * Course: CS3413
 * Assignment #: 1
 * Compilation: gcc -Wall -o a1 a1.c
 * Date: Jan 16, 2021
 *
 * This file is source code for the solution to Assignment 1 of CS2413
 * The solution is strictly limited to a single file to comply with the
 *  assignment specifications. The solution assumes that the input data
 *  isn't necessarily formatted in chronological order
 * Source code is formatted with clang-format using the Google style guide
 *
 * clang-format on
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define DELIMETERS " \t"

// ---------------- Global Error Handling ----------------
/**
 * @brief exits the program with provided error number after printing error
 * number and associated Unix error message, from strerror, or a message
 * indicating errorNumber is unknown to strerror
 *
 * @param errorNumber Unix error number
 */
void handleExitError(const int errorNumber) {
  const char *errorString = strerror(errorNumber);
  if (errorString == (char *)NULL) {
    errorString =
        "Invalid error number. Cannot deduce error message from error number";
  }
  fprintf(stderr, "error = %i - %s\n", errorNumber, errorString);
  exit(errorNumber);
}

// ---------------- String Util ----------------
/**
 * @brief constructs a new string on the heap with the same contents as the
 * original
 *
 * @param original
 * @return char*
 */
char *string_copyConstruct(const char *original) {
  if (original == (char *)NULL) {
    fprintf(stderr,
            "string_copyConstruct requires 'original' field to point to a "
            "valid address\n");
    return (char *)NULL;
  }
  void *const newStringBuffer = malloc(strlen(original) + 1);
  if (newStringBuffer == (void *)NULL) {
    handleExitError(errno);
  }
  char *const newString = (char *)newStringBuffer;
  strcpy(newString, original);
  return newString;
}

// ---------------- Job Requests From Input ----------------
/**
 * @brief represents the fields of a job request from the input and stores the
 * remaining duration the job has until completion
 */
typedef struct JobRequest {
  unsigned remainingDuration;
  // job request fields:
  const char *user;
  char process;
  unsigned arrival;
  unsigned duration;
} JobRequest;

/**
 * @brief represents the result of parsing a line from stdin, where job request
 * contains the fields described in the input and valid represents if these were
 * properly parsed
 *
 */
typedef struct JobRequestValid {
  JobRequest *jobRequest;
  bool valid; // validity of job request fields
} JobRequestValid;

/**
 * @brief Parses the input line describing a job request with fields
 * delimited by 'delimeter' and returns the concrete fields in a JobRequest
 * object
 *
 * @param inputLine string containing single job request row
 * @param delimeter delimiting character used to separate fields
 * @return JobRequest with concrete data fields from inputLine or arbitrary
 * data if valid flag is false. JobRequest owns all members
 */
JobRequestValid parseInputLine(char *const inputLine,
                               const char *const delimeters) {
  JobRequestValid result = {.jobRequest = (JobRequest *)NULL, .valid = false};
  // Argument validity checks
  if (inputLine == (char *)NULL) {
    fprintf(
        stderr,
        "field 'inputLine' of parseInputLine must point to a valid address");
    return result;
  }
  if (delimeters == (char *)NULL) {
    fprintf(
        stderr,
        "field 'delimeters' of parseInputLine must point to a valid address");
    return result;
  }

  void *jobRequestBuffer = malloc(sizeof(JobRequest));
  if (jobRequestBuffer == (void *)NULL) {
    handleExitError(errno);
  }
  result.jobRequest = (JobRequest *)jobRequestBuffer;
  result.valid = true;

  // 1. User field parsing
  const char *const userName = strtok(inputLine, delimeters);
  if (userName == (char *)NULL) {
    fprintf(
        stderr,
        "Failure to parse 'user' while processing input line describing job "
        "request. Ignoring job request\n");
    result.valid = false;
    return result;
  }
  result.jobRequest->user = string_copyConstruct(userName);

  // 2. Process field parsing
  const char *processName = strtok(NULL, delimeters);
  if (processName == (char *)NULL) {
    fprintf(
        stderr,
        "Failure to parse 'process' while processing input line describing job "
        "request for user %s. Ignoring job request\n",
        userName);
    result.valid = false;
    return result;
  }
  if (strlen(processName) > 1) { // expect only single char
    fprintf(stderr,
            "Invalid process name (%s) for job request for user %s. Ignoring "
            "job request\n",
            processName, userName);
    result.valid = false;
    return result;
  }
  result.jobRequest->process = *processName;

  // 3. Arrival field parsing
  const char *const arrivalString = strtok(NULL, delimeters);
  if (arrivalString == (char *)NULL) {
    fprintf(
        stderr,
        "Failure to parse 'arrival' while processing input line describing job "
        "request with name %s for user %s. Ignoring job request\n",
        processName, userName);
    result.valid = false;
    return result;
  }
  // convert from string to (long) integer
  const unsigned long arrivalConversion =
      strtoul(arrivalString, (char **)NULL, 10);
  if (errno != 0) {
    fprintf(
        stderr,
        "Failure to convert the arrival field in the input string describing "
        "process %s to a long integer. Ignoring job request\n",
        processName);
    result.valid = false;
    return result;
  }
  // cast to result.arrival field type, unsigned int, if well-defined
  if (arrivalConversion > UINT_MAX) {
    fprintf(stderr,
            "Arrival field of job request %s is too large. The max size is %i",
            processName, UINT_MAX);
    result.valid = false;
    return result;
  }
  result.jobRequest->arrival = (unsigned)arrivalConversion;

  // 4. Duration parsing
  const char *const durationString = strtok(NULL, delimeters);
  if (durationString == (char *)NULL) {
    fprintf(stderr,
            "Failure to parse 'duration' while processing input line "
            "describing job "
            "request with name %s for user %s",
            processName, userName);
    result.valid = false;
    return result;
  }
  // convert from string to (long) integer
  const unsigned long durationConversion =
      strtoul(durationString, (char **)NULL, 10);
  if (errno != 0) {
    fprintf(
        stderr,
        "Failure to convert the duration field in the input string describing "
        "process %s to a long integer. Ignoring job request\n",
        processName);
    result.valid = false;
    return result;
  }
  //  cast to result.duration field type, unsigned int, if well-defined
  if (durationConversion > UINT_MAX) {
    fprintf(stderr,
            "Duration field of job request %s is too large. The max size is %i",
            processName, UINT_MAX);
    result.valid = false;
    return result;
  }
  result.jobRequest->duration = (unsigned)durationConversion;
  result.jobRequest->remainingDuration = result.jobRequest->duration;

  return result;
}

// ---------------- Summary ----------------
/**
 * @brief represents rows of summary table; one for each user
 *
 */
typedef struct UserSummary {
  const char *user;
  unsigned lastCompletedTime;
} UserSummary;

// ---------------- Linked List ----------------
/**
 * @brief represents nodes of linked list
 *
 */
typedef struct LinkedNode {
  const void *data;
  struct LinkedNode *next;
} LinkedNode;

/**
 * @brief basic handle for linked list container
 *
 */
typedef struct LinkedList {
  LinkedNode *head;
} LinkedList;

/**
 * @brief returns data contents of LinkedNode casted to a JobRequest*
 *
 * @param node LinkedNode containing data
 * @return JobRequest* contained data
 */
JobRequest *linkedNode_jobRequest(const LinkedNode *const node) {
  return node == (LinkedNode *)NULL ? NULL : (JobRequest *)node->data;
}

/**
 * @brief Compares arrival times between JobRequests of LinkedNodes through
 * subtraction
 *
 * @param first LinkedNode to compare. Minuend
 * @param second LinkedNode to compare. Subtrahend
 * @return long difference between arrival times
 */
long linkedNode_compareArrival(const LinkedNode *const first,
                               const LinkedNode *const second) {
  // Argument validity checks
  if (first == (LinkedNode *)NULL) {
    fprintf(stderr, "argument 'first' of linkedNode_compareArrival must point "
                    "to a valid address\n");
    return 0;
  }
  if (second == (LinkedNode *)NULL) {
    fprintf(stderr, "argument 'second' of linkedNode_compareArrival must point "
                    "to a valid address\n");
    return 0;
  }
  // Comparison
  const JobRequest *const firstJob = linkedNode_jobRequest(first);
  const JobRequest *const secondJob = linkedNode_jobRequest(second);
  return (long)firstJob->arrival - (long)secondJob->arrival;
}

/**
 * @brief frees the LinkedNode containing a JobRequest and the LinkedNode's
 * JobRequest data that were both allocated on the heap. Does not free the
 * JobRequest's user string
 *
 * @param node
 */
void linkedNode_free(const LinkedNode *const node) {
  if (node == (LinkedNode *)NULL) {
    return;
  }
  JobRequest *job = (JobRequest *)node->data;
  free(job);
  free((LinkedNode *)node);
}

/**
 * @brief pushes a JobRequest into a linked list in location where ascending
 * order between the JobRequests' arrival times is maintained
 *
 * @param list LinkedList to push into
 * @param job JobRequest to push into list
 * @return true push was successful
 * @return false push was unsuccessful; an error occurred
 */
bool linkedList_pushOrdered(LinkedList *const list,
                            const JobRequest *const job) {
  if (list == (LinkedList *)NULL) {
    fprintf(stderr,
            "linkedList_pushOrdered requires that the 'list' parameter points "
            "to a valid LinkedList\n");
    return false;
  }
  if (job == (JobRequest *)NULL) {
    fprintf(stderr,
            "linkedList_pushOrdered requires that the 'job' parameter points "
            "to a valid JobRequest\n");
    return false;
  }

  // Create new node with job to push into list
  void *newNodeBuffer = malloc(sizeof(LinkedNode));
  if (newNodeBuffer == (void *)NULL) {
    fprintf(stderr, "Failure trying to allocate memory for list node in "
                    "linkedList_pushOrdered\n");
    handleExitError(errno);
  }
  LinkedNode *newNode = (LinkedNode *)newNodeBuffer;
  newNode->data = job;

  // Find appropriate spot in list to insert new node such that ordered is kept
  if (list->head == (LinkedNode *)NULL) { // empty list
    list->head = newNode;
    newNode->next = (LinkedNode *)NULL;
  } else if (linkedNode_compareArrival(newNode, list->head) <=
             0) { // push front of list
    newNode->next = list->head;
    list->head = newNode;
  } else { // push in or end of list
    LinkedNode *currentNode = list->head;
    while (currentNode->next != (LinkedNode *)NULL &&
           linkedNode_compareArrival(currentNode->next, newNode) < 0) {
      currentNode = currentNode->next;
    }
    newNode->next = currentNode->next;
    currentNode->next = newNode;
  }
  return true;
}

/**
 * @brief pushes a new LinkedNode containing supplied data onto back of provided
 * LinkedList
 *
 * @param list LinkedList to push data to
 * @param data data desired data to push onto list
 * @return true successfully pushed data to list
 * @return false unsuccessful; error occurred; did not complete push
 */
bool linkedList_push(LinkedList *const list, const void *const data) {
  if (list == (LinkedList *)NULL) {
    fprintf(stderr, "linkedList_push requires that the 'list' parameter points "
                    "to a valid LinkedList\n");
    return false;
  }
  if (data == (void *)NULL) {
    fprintf(stderr, "linkedList_push requires that the 'data' parameter points "
                    "to a valid address\n");
    return false;
  }

  // Create new node with job to push into list
  void *newNodeBuffer = malloc(sizeof(LinkedNode));
  if (newNodeBuffer == (void *)NULL) {
    fprintf(stderr, "Failure trying to allocate memory for list node in "
                    "linkedList_push\n");
    handleExitError(errno);
  }
  LinkedNode *newNode = (LinkedNode *)newNodeBuffer;
  newNode->data = data;
  newNode->next = (LinkedNode *)NULL;

  // Insert node at end of list
  if (list->head == (LinkedNode *)NULL) { // empty list
    list->head = newNode;
  } else {
    LinkedNode *secondLast = list->head;
    while (secondLast->next != (LinkedNode *)NULL) {
      secondLast = secondLast->next;
    }
    secondLast->next = newNode;
  }
  return true;
}

// null if unable to remove
/**
 * @brief Removes provided data from desired LinkedList
 *
 * @param list LinkedList to remove data from
 * @param data arbitrary data stored in list to remove
 * @return LinkedNode* the node that contained data and was removed, or NULL if
 * error occurs or data doesn't exists. Must be freed
 */
LinkedNode *linkedList_remove(LinkedList *const list, const void *const data) {
  // Argument validity checks
  if (list == (LinkedList *)NULL) {
    fprintf(stderr,
            "list in linkedList_remove must point to valid LinkedList\n");
    return (LinkedNode *)NULL;
  }
  if (data == (void *)NULL) {
    fprintf(stderr,
            "data in linkedList_remove must point to a valid address\n");
    return (LinkedNode *)NULL;
  }
  // Head node contains desired data
  if (list->head != (LinkedNode *)NULL && list->head->data == data) {
    LinkedNode *desiredNode = list->head;
    list->head = desiredNode->next;
    return desiredNode;
  }
  // Move to node before desired node
  LinkedNode *current = list->head;
  while (current != (LinkedNode *)NULL && current->next != (LinkedNode *)NULL &&
         current->next->data != data) {
    current = current->next;
  }
  // Node not found
  if (current == (LinkedNode *)NULL || current->next == (LinkedNode *)NULL) {
    return (LinkedNode *)NULL;
  }
  // Remove node from list
  LinkedNode *desiredNode = current->next;
  current->next = current->next->next;
  return desiredNode;
}

/**
 * @brief Updates the UserSummary time data that matches the supplied user name
 * in the supplied LinkedList containing UserSummary data. Only updates time if
 * it is greater than the existing time
 *
 * @param summary LinkedList storing total Summary
 * @param user the name of the user to update
 * @param time time to update user summary to
 * @return true successfully updated
 * @return false unsuccessful; errors occurred or supplied time was <= existent
 * time
 */
bool linkedList_updateLastCompletedTime(LinkedList *summary,
                                        const char *const user,
                                        const unsigned time) {
  // Argument validity checks
  if (user == (char *)NULL) {
    fprintf(stderr, "field 'user' to linkedList_updateLastCompleteTime must "
                    "point to valid address\n");
    return false;
  }
  if (summary == (LinkedList *)NULL) {
    fprintf(stderr, "field 'summary' to linkedList_updateLastCompleteTime must "
                    "point to valid address\n");
    return false;
  }

  // Update
  const LinkedNode *current;
  for (current = summary->head; current != (LinkedNode *)NULL;
       current = current->next) {
    UserSummary *const userSummary = ((UserSummary *)current->data);
    if (strcmp(userSummary->user, user) == 0) {
      if (time > userSummary->lastCompletedTime) {
        userSummary->lastCompletedTime = time;
        return true;
      }
    }
  }
  return false;
}

// ---------------- Job Submission ----------------
/**
 * @brief Effectively submits supplied job to CPU by printing it to stdout. Sets
 * errno on failure
 *
 * @param submissionTime current time to submit job
 * @param jobRequests all job requests that contains job
 * @param job job to submit
 * @return LinkedNode* the removed node if the job completed or null otherwise
 * (Error or not complete). Must be freed
 */
LinkedNode *submitJob(const unsigned submissionTime,
                      LinkedList *const jobRequests, JobRequest *const job) {
  // Argument validity checks
  if (jobRequests == (LinkedList *)NULL) {
    fprintf(stderr,
            "field 'jobRequests' of submitJob must point to valid address\n");
    errno = EPERM;
    return (LinkedNode *)NULL;
  }
  if (job == (JobRequest *)NULL) {
    fprintf(stderr, "field 'job' of submitJob must point to valid address\n");
    errno = EPERM;
    return (LinkedNode *)NULL;
  }
  // Job Submission
  --(job->remainingDuration);
  printf("%u\t%c\n", submissionTime, job->process);
  if (job->remainingDuration == 0) {
    LinkedNode *const removed = linkedList_remove(jobRequests, job);
    if (removed == (LinkedNode *)NULL) {
      fprintf(stderr, "Cannot proceed without clearing completed job");
      handleExitError(EPERM);
    }
    errno = 0;
    return removed;
  }
  errno = 0;
  return (LinkedNode *)NULL;
}

/**
 * @brief Effectively idles the CPU by printing to stdout
 *
 * @param submissionTime
 */
void idleCpu(const unsigned submissionTime) {
  printf("%u\tIDLE\n", submissionTime);
}

JobRequest *shortestJob(LinkedList *const jobRequests,
                        const unsigned currentTime) {
  // Argument validity check
  if (jobRequests == (LinkedList *)NULL) {
  }
  if (jobRequests->head == (LinkedNode *)NULL) {
    return (JobRequest *)NULL;
  }
  // Find shortest duration job that arrived on or before current time
  const LinkedNode *node = jobRequests->head;
  JobRequest *job = linkedNode_jobRequest(node);
  JobRequest *shortestJob = job;
  JobRequest *nextJob = linkedNode_jobRequest(jobRequests->head->next);
  while (node->next != (LinkedNode *)NULL && nextJob->arrival <= currentTime) {
    if (nextJob->duration < job->duration) {
      shortestJob = nextJob;
    }
    node = node->next;
    job = linkedNode_jobRequest(node);
    nextJob = linkedNode_jobRequest(node->next);
  }
  return shortestJob;
}

// ---------------- Main ----------------
int main() {
  // 1. Read Input
  // Skip title row of input data
  fscanf(stdin, "%*[^\n]\n");
  if (errno != 0) {
    handleExitError(errno);
  }

  // Buffer to store each input line
  size_t inputLineLength = 20;
  void *inputLineBuffer = malloc(sizeof(char) * inputLineLength);
  if (inputLineBuffer == (void *)NULL) {
    free(inputLineBuffer);
    handleExitError(errno);
  }
  char *inputLine = (char *)inputLineBuffer;
  ssize_t lineSize;

  // Parse JobRequest from each line and add to list of JobRequests
  //  add names to summary
  LinkedList jobRequests = {.head = NULL};
  LinkedList summary = {.head = NULL};
  while (true) {
    // read input line
    lineSize = getline(&inputLine, &inputLineLength, stdin);
    if (errno != 0) {
      free(inputLineBuffer);
      handleExitError(errno);
    }
    if (lineSize == -1) { // end-of-file
      break;
    }
    // parse input line
    const JobRequestValid parsedRequest = parseInputLine(inputLine, DELIMETERS);
    if (!parsedRequest.valid) {
      fprintf(stderr,
              "Error while parsing input line %s. Ignoring this job request\n",
              inputLine);
      continue;
    }
    // push job request
    JobRequest *jobRequest = parsedRequest.jobRequest;
    const bool pushSuccess = linkedList_pushOrdered(&jobRequests, jobRequest);
    if (!pushSuccess) {
      fprintf(stderr,
              "Unable to store job request %c for user %s. Ignoring this "
              "request.\n",
              jobRequest->process, jobRequest->user);
    }
    // push user to summary if they're new
    bool newUser = true;
    const LinkedNode *current;
    for (current = summary.head; current != (LinkedNode *)NULL;
         current = current->next) {
      const char *const currentName = ((UserSummary *)current->data)->user;
      if (strcmp(currentName, jobRequest->user) == 0) {
        newUser = false;
        break;
      }
    }
    if (newUser) {
      // create string for user summary and copy user name from inputLine
      const char *const userSummaryString =
          string_copyConstruct(jobRequest->user);

      void *userSummaryBuffer = malloc(sizeof(UserSummary));
      if (userSummaryBuffer == (void *)NULL) {
        free(inputLineBuffer);
        handleExitError(errno);
      }
      UserSummary *const userSummary = (UserSummary *)userSummaryBuffer;
      userSummary->user = userSummaryString;
      userSummary->lastCompletedTime = 0;
      linkedList_push(&summary, userSummary);
    }
  }

  // 2. Submit jobs in SJF order
  printf("Time\tJob\n");                        // title line
  if (jobRequests.head != (LinkedNode *)NULL) { // JobRequests contains elements
    JobRequest *job = linkedNode_jobRequest(jobRequests.head);
    unsigned time;
    // iterate through time from first job to last job
    for (time = job->arrival; job != (JobRequest *)NULL; ++time) {
      if (job->arrival <= time) {
        // find job to schedule
        JobRequest *const shortest = shortestJob(&jobRequests, time);
        if (shortest == (JobRequest *)NULL) {
          free(inputLineBuffer);
          handleExitError(EPERM);
        }
        // schedule job to CPU
        LinkedNode *const removed = submitJob(time, &jobRequests, shortest);
        if (errno != 0) {
          handleExitError(errno);
        }
        if (removed != (LinkedNode *)NULL) { // completed job
          const bool updated = linkedList_updateLastCompletedTime(
              &summary, shortest->user, time);
          if (!updated) {
            fprintf(stderr, "Error while updating summary for user %s\n",
                    shortest->user);
          }
          free((char *)shortest->user);
          linkedNode_free(removed);
        }
        job = linkedNode_jobRequest(jobRequests.head);
      } else { // idle while CPU waits for next job to 'arrive'
        idleCpu(time);
      }
    }
    idleCpu(time);

    // 3. Print user summaries of job submissions and free user summaries
    printf("\nSummary\n");
    const LinkedNode *current;
    const LinkedNode *next;
    for (current = summary.head, next = summary.head;
         current != (LinkedNode *)NULL; current = next) {
      const UserSummary *const userSummary = ((UserSummary *)current->data);
      printf("%s\t%u\n", userSummary->user, userSummary->lastCompletedTime + 1);
      next = current->next;
      free((char *)userSummary->user);
      free((UserSummary *)userSummary);
      free((LinkedNode *)current);
    }
  }
  free(inputLine);
  return EXIT_SUCCESS;
}
