/*
 *      This file is part of the Smoke project.
 *
 *  Copyright of this program is the property of its author(s), without
 *  those written permission reproduction in whole or in part is prohibited.
 *  More details on the LICENSE file delivered with the project.
 *
 *   - - - - - - - - - - - - - - -
 *
 *      Header file of the asynchronous module
 *      Handle all asynchronous jobs and events.
 */
#ifndef KERNEL_ASYNC_H__
#define KERNEL_ASYNC_H__

#include <kernel/core.h>

// ===========================================================================
//      Definitions and Macros
// ===========================================================================

// ===========================================================================
//      Data Structures
// ===========================================================================

// ---------------------------------------------------------------------------
/** Structure for task that are waiting an asynchrone event.
  * @note Some threads may not be waiting, in this case they receive a signal.
  */
struct kWaiting
{
  int         handle_;    ///< Handle for user application and canceling/abort.
  kThread_t*  task_;      ///< Thread currently waiting.
  int         reason_;    ///< Reason of the wait (EV_*).
  nanotime_t  timeout_;   ///< Time before canceling.
  long        param_;     ///< Param depending of the reason.
  llnode_t    waitNd_;    ///< Node for all waiting objects.
  llnode_t    targetNd_;  ///< Node to link to the trigering object.
  llhead_t*   target_;    ///< Save the list of the target to remove the node.
};


// ---------------------------------------------------------------------------
/** */
struct kEvent 
{
  char size_;
  char type_;

  union {

    struct {
      char rel_x_;
      char rel_y_;
    } mouse_motion_;

    struct {
      short button_;
    } mouse_button_;

    struct {
      short key_;
    } keyboard_;

  };
};


// ---------------------------------------------------------------------------
/** */
enum EV_Reason
{
  EV_UNKNOW = 0, 
  EV_SLEEP,       ///< Sleep a defined amount of time
  EV_INTERVAL,    ///< Sleep a repeated amount of time (param period).
  EV_READ,        ///< Wait for to have at least x bytes to read, (or '\n').
  EV_EXIT,        ///< Wait for a process to exit (param pid)
  EV_ASYNC,       ///< Wait until an async worker finish.
};


// ===========================================================================
//      Methods
// ===========================================================================

// ASYNC/WAIT ================================================================
/** Check the time of registers events */
void async_ticks ();
/** Register to an event */
int async_event(kThread_t* task, llhead_t* targetList, int reason, long param, long maxtime);
/** Wake up the task waiting for an event */
void async_wakeup (kWaiting_t* wait);
/** Cancel an event */
void async_cancel_event (kThread_t* task);
/** */
void async_trigger (llhead_t* targetList, int reason, long param);

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

#endif /* KERNEL_ASYNC_H__ */
