///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2011-2017, PALANDesign Hannover, Germany
//
// \license The MIT License (MIT)
//
// This file is part of the decom library.
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// \brief Scheduler class
//
// This class implements a universal message scheduler.
// - Messages are organized in tables.
// - Tables are organized in lists.
// - One list is activated and looped.
//
// Example:
//
//     table 0          table 1          table 2
//  |-----------|    |-----------|    |-----------|
//  |   msg 10  |    |   msg 4   |    |   msg 8   |
//  |   msg 3   |    |   msg 1   |    |   msg 11  |
//  |   msg 7   |    |   msg 2   |    |   msg 2   |
//  |-----------|    |-----------|    |-----------|
//
//     list 0           list 1           list 2
//  |-----------|    |-----------|    |-----------|
//  |  table 0  |    |  table 1  |    |  table 1  |
//  |  table 1  |    |  table 0  |    |  table 2  |
//  |  table 2  |    |  table 3  |    |           |
//  |-----------|    |-----------|    |-----------|
//       ^
//       active list
//
// The configuration above outputs the following messages:
// 10, 3, 7, 4, 1, 2, 8, 11, 2, 10, 3, 7, ...
// - tab 1 - -tab2 -  -tab 3 -  - tab 1 -
// ----------- list 1 --------  - loop from begin ...
//
// In periodic protocols like LIN or CAN every message has its own ID.
// To identify the messages by their ID the channel parameter is used by the scheduler.
// Additional to table/list mechanism a simple periodic message looper may be
// used via set_periodic_message() function
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_PROT_SCHEDULER_H_
#define _DECOM_PROT_SCHEDULER_H_

#include <map>
#include <vector>

#include "prot.h"

/////////////////////////////////////////////////////////////////////

namespace decom {
namespace prot {

class scheduler : public protocol
{
public:
  /**
   * Normal protocol ctor
   * \param Lower layer
   */
  scheduler(decom::layer* lower, const char* name = "prot_scheduler");

  /**
   * dtor
   */
  virtual ~scheduler();

  /**
   * Called by upper layer to open this layer
   * \param address The address to open
   * \param id The endpoint identifier to open
   * \return true if open is successful
   */
  virtual bool open(const char* address = "", decom::eid const& id = eid_any);

  /**
   * Called by upper layer to close this layer
   * \param id The endpoint identifier to close
   */
  virtual void close(decom::eid const& id = eid_any);

  /**
   * Called by upper layer to transmit data (message) to this protocol
   * \param data The message to send
   * \param id The endpoint identifier
   * \param more true if message is a fragment which is followed by another msg. Ignored here
   * \return true if Send is successful
   */
  virtual bool send(decom::msg& data, decom::eid const& id = eid_any, bool more = false);


  ////////////////////////////////////////////////////////////////////////
  // L A Y E R   A P I

  /**
   * Add a message to the according schedule table
   * Tables MUST BE created in ascending order (0,1,2...)
   * \param msg_id The eid of the message to add to the table
   * \param table_id The ID of the table
   * \return true if message was added successfully
   */
  bool add_message(decom::eid msg_id, std::int32_t table_id);

  /**
   * Add a table to the according schedule list
   * Lists MUST BE created in order (0,1,2...)
   * \param table_id The ID of the table to add to the list
   * \param list_id The ID of the list
   * \return true if table was added successfully
   */
  bool add_table(std::int32_t table_id, std::int32_t list_id);

  /**
   * Set the given list ID as next scheduled list, after the current list is finished
   * \param list_id The ID of the list
   * \return true if list is activated successfully
   */
  bool activate_list(std::int32_t list_id);

  /**
   * Inject the given table in the current scheduling
   * The given table is injected in scheduling after the current active table ended.
   * \param table_id The ID of the table
   * \return true if successful
   */
  bool inject_table(std::int32_t table_id);

  /**
   * Define a message which is sent periodically to the lower layer - independent of table/list/queue scheduling.
   * A message which is send to the scheduler is stored and sent when the message timer elapsed.
   * \param msg_id The eid of the message to send
   * \param interval The interval time of the message in [ms]
   *        If interval is 0, the message is removed from the scheduler.
   * \return true if successful
   */
  bool set_periodic_message(decom::eid msg_id, std::chrono::milliseconds interval);


  /**
   * Start scheduling messages
   * \param reset_scheduler True to reset the scheduler and begin with the start of the main queue
   */
  bool start(bool scheduler_reset = true);

  /**
   * Stop scheduling messages
   */
  bool stop();


  /**
   * Set the scheduler base period time.
   * This time defines the interval between two messages (smallest interval) in the schedule table.
   * All other timing values are multiple units of this value.
   * \param interval The interval length
   * \return true if successful
   */
  bool set_scheduler_period(std::chrono::milliseconds interval);

  /**
   * Set the callback function which is called after a message has been send to lower layer.
   * This can be used as notification function or e.g. for message injection
   * \param arg Argument which is passed to callback function
   * \param fn_post_sent Callback function
   * \param msg_id ID of the message which was sent
   * \param table_id ID of the table of the message
   * \param list_id ID of the list of the table
   */
  void set_scheduler_callback(void* arg, void(*fn_post_sent)(void* arg, std::int32_t msg_id, std::int32_t table_id, std::int32_t list_id));

private:
  bool is_open_;       // true if layer is open

  std::chrono::milliseconds scheduler_period_;        // base period time of the scheduler
  bool scheduler_reset_;                              // true to reset/init the scheduler logic

  typedef struct struct_message_type {
    decom::msg data;                                  // message data
    std::chrono::milliseconds interval;               // message interval time in [ms]
    std::chrono::milliseconds elapsed;                // elapsed time since last transmission
  } message_type;

  std::map<eid, message_type>             messages_;  // map which contains all messages
  std::vector<std::vector<decom::eid> >   tables_;    // vector of all tables
  std::vector<std::vector<std::int32_t> > lists_;     // vector of all lists

  std::int32_t inject_table_id_;                      // the table ID which is injected as next
  std::int32_t next_list_id_;                         // next scheduled list

  std::mutex msg_mutex_;                              // msg lock for updates
  decom::util::timer timer_;                          // main timer

  void (*callback_)(void* arg, std::int32_t msg_id, std::int32_t table_id, std::int32_t list_id); // scheduler callback function
  void* callback_arg_;                                // callback arg

  static void timer_callback(void* arg);
};

} // namespace prot
} // namespace decom

#endif // _DECOM_PROT_SCHEDULER_H_
