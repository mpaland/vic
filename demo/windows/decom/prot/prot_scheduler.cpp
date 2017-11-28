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
///////////////////////////////////////////////////////////////////////////////

#include "prot_scheduler.h"

namespace decom {
namespace prot {

scheduler::scheduler(decom::layer* lower, const char* name)
 : protocol(lower, name)    // it's VERY IMPORTANT to call the base class ctor HERE!!!
{
  is_open_ = false;

  inject_table_id_ = -1;
  next_list_id_    = 0;

  callback_     = nullptr;
  callback_arg_ = nullptr;
}


scheduler::~scheduler()
{
  is_open_ = false;
}


// called by upper layer
bool scheduler::open(const char* address, decom::eid const& id)
{
  // for security: check that upper protocol/device exists
  if (!upper_) {
    return false;
  }

  // open the lower layer first - opening is done down-top in layer stack
  // if opening the lower layer fails, report it
  is_open_ = protocol::open(address, id);

  return is_open_;
};


void scheduler::close(decom::eid const& id)
{
  // close this layer here
  is_open_ = false;

  // stop the scheduler
  stop();

  // close the lower layer - closing is done top-down in layer stack
  protocol::close(id);
}


bool scheduler::send(decom::msg& data, decom::eid const& id, bool)
{
  // check if the message is in the message table
  // if not pass it directly and independently of the scheduler to the lower layer
  std::map<decom::eid, message_type>::iterator it = messages_.find(id);
  if (it != messages_.end()) {
    // msg found in table, update data
    msg_mutex_.lock();
    it->second.data = data; // use a cheap copy
    msg_mutex_.unlock();
    return true;
  }

  // send the message to lower layer and return result
  return protocol::send(data, id);
}


bool scheduler::add_message(decom::eid msg_id, std::int32_t table_id)
{
  if (table_id > static_cast<std::int32_t>(tables_.size())) {
    // out of range
    return false;
  }

  // create new table
  if (table_id == static_cast<std::int32_t>(tables_.size())) {
    std::vector<decom::eid> table;
    tables_.push_back(table);
  }

  // add message to table
  tables_[table_id].push_back(msg_id);

  // insert message
  std::map<decom::eid, message_type>::iterator it = messages_.find(msg_id);
  if (it == messages_.end()) {
    message_type msg;
    msg.interval = std::chrono::milliseconds(0);
    messages_.insert(std::pair<decom::eid, message_type>(msg_id, msg));
  }

  return true;
}


bool scheduler::add_table(std::int32_t table_id, std::int32_t list_id)
{
  if (table_id >= static_cast<std::int32_t>(tables_.size())) {
    // out of range
    return false;
  }

  if (list_id > static_cast<std::int32_t>(lists_.size())) {
    // out of range
    return false;
  }

  // create new list
  if (list_id == static_cast<std::int32_t>(lists_.size())) {
    std::vector<std::int32_t> list;
    lists_.push_back(list);
  }

  // add table to list
  lists_[list_id].push_back(table_id);

  return true;
}


bool scheduler::activate_list(std::int32_t list_id)
{
  if (lists_.empty() || list_id > static_cast<std::int32_t>(lists_.size())) {
    // out of range
    return false;
  }

  next_list_id_ = list_id;
  return true;
}


bool scheduler::inject_table(std::int32_t table_id)
{
  if (table_id > (int)tables_.size()) {
    // out of range
    return false;
  }
  if (inject_table_id_ != -1) {
    // table injection already in progress
    return false;
  }

  inject_table_id_ = table_id;
  return true;
}


bool scheduler::set_periodic_message(decom::eid msg_id, std::chrono::milliseconds interval)
{
  message_type msg;
  msg.interval = interval;
  msg.elapsed  = std::chrono::milliseconds(0);

  std::map<decom::eid, message_type>::iterator it = messages_.find(msg_id);
  if (it == messages_.end()) {
    // insert message
    messages_.insert(std::pair<decom::eid, message_type>(msg_id, msg));
  }
  else {
    // update the message
    it->second.interval = interval;
  }

  return true;
}


bool scheduler::start(bool scheduler_reset)
{
  // checks
  if (tables_.empty() || lists_.empty()) {
    DECOM_LOG_WARN("No tables or lists defined - only periodic messsages will work");
  }
  if (!is_open_) {
    DECOM_LOG_ERROR("Layer not open, scheduler can't start");
    return false;
  }

  scheduler_reset_ = scheduler_reset;

  bool result = timer_.start(scheduler_period_, true, &scheduler::timer_callback, this);
  if (result) {
    DECOM_LOG_INFO("Scheduler started");
  }

  return result;
}


bool scheduler::stop()
{
  bool result = timer_.stop();
  if (result) {
    DECOM_LOG_INFO("Scheduler stopped");
  }

  return result;
}


bool scheduler::set_scheduler_period(std::chrono::milliseconds interval)
{
  scheduler_period_ = interval;

  return true;
}


void scheduler::set_scheduler_callback(void* arg, void(*fn_post_sent)(void* arg, std::int32_t msg_id, std::int32_t table_id, std::int32_t list_id))
{
  callback_     = fn_post_sent;
  callback_arg_ = arg;
}


void scheduler::timer_callback(void* arg)
{
  scheduler* s = static_cast<scheduler*>(arg);

  static std::int32_t msg_idx   = -1;       // current scheduled message i (init with -1 due to increment)
  static std::int32_t table_idx = 0;        // current scheduled table
  static std::int32_t list_idx  = 0;        // current scheduled list
  static std::int32_t table_idx_last = -1;  // last regular scheduled table before injection (-1 = none)

  if (s->scheduler_reset_) {
    msg_idx = table_idx_last = -1;          // reset scheduling to startup condition
    table_idx = list_idx = 0;
    s->scheduler_reset_ = false;
  }

  // timer service only if layer is open
  if (!s->is_open_) return;

  // calculate the next message
  if (s->tables_.size() && s->lists_.size() && ++msg_idx >= static_cast<std::int32_t>(s->tables_[table_idx].size())) {
    // table end reached
    msg_idx = 0;

    // restore from table injection ?
    if (table_idx_last != -1) {
      // yes, restore last table
      table_idx           = table_idx_last;
      table_idx_last      = -1;
      s->inject_table_id_ = -1;
    }

    // check if a table should be injected
    if (s->inject_table_id_ != -1) {
      // yes, table injection
      table_idx_last = table_idx;
      table_idx      = s->inject_table_id_;
    }
    else {
      // no, just get next table in list
      if (++table_idx >= static_cast<std::int16_t>(s->lists_[list_idx].size())) {
        // list end reached
        table_idx = 0;
      }
    }

    std::map<decom::eid, message_type>::iterator it = s->messages_.find(s->tables_[s->lists_[list_idx][table_idx]][msg_idx]);
    if (it != s->messages_.end()) {
      // msg found, get data
      s->msg_mutex_.lock();
      decom::msg data = it->second.data;   // data may be clothed on lower layer, so use a real copy here
      s->msg_mutex_.unlock();
      s->protocol::send(data, it->first);
      if (s->callback_) {           // callback
        s->callback_(s->callback_arg_, msg_idx, table_idx, list_idx);
      }
    }
  }

  // handle periodic messages
  for (std::map<decom::eid, message_type>::iterator it = s->messages_.begin(); it != s->messages_.end(); ++it) {
    if (it->second.interval > std::chrono::milliseconds(0)) {
      it->second.elapsed += s->scheduler_period_;
      if (it->second.elapsed >= it->second.interval) {
        it->second.elapsed = std::chrono::milliseconds(0);
        s->msg_mutex_.lock();
        decom::msg data = it->second.data;    // data may be clothed on lower layer, so use a real copy here
        s->msg_mutex_.unlock();
        s->protocol::send(data, it->first);
      }
    }
  }
}

} // namespace prot
} // namespace decom
