///////////////////////////////////////////////////////////////////////////////
// \author (c) Marco Paland (info@paland.com)
//             2012-2017, PALANDesign Hannover, Germany
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
// \brief message class
//
// This class provides the message buffer class and the buffer pool
//
//   ------------------------------------ m s g ------------------------------------
//   -----------------              -----------------              -----------------
//   |               |     next     |               |     next     |               | next = nullptr
//   |    P A G E    | -----------> |    P A G E    | -----------> |    P A G E    |-------->
//   |               |              |               |              |               |
//   -----------------              -----------------              -----------------
//     ^         ^                  ^              ^               ^       ^
//     |head     |tail              |head          |tail           |head   |tail
//
// A page is a unit of a msg and has a head and a tail pointer. If head == tail the page is empty.
// The next pointer points to the next page in the chain or is nullptr on last page.
// ref is a counter of how many references a page/msg has. If a page has more than one reference it
// is read only, all operations which change the msg content are prohibited then.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _DECOM_MSG_H_
#define _DECOM_MSG_H_

#include <cstdint>
#include <mutex>
#include <iterator>
#include <cstring>    // for memcpy

#include "decom_cfg.h"
#include "log.h"

///////////////////////////////////////////////////////////////////////////////

// defines the page size in bytes
// you may modify this value according to your belongings
#ifndef DECOM_MSG_POOL_PAGE_SIZE
#define DECOM_MSG_POOL_PAGE_SIZE    128U
#endif

// defines the number of pages in the pool
// POOL_PAGE_SIZE * POOL_PAGES is the total required memory size
#ifndef DECOM_MSG_POOL_PAGES
#define DECOM_MSG_POOL_PAGES        128U
#endif

// defines the start offset of a new message within the first page
// set this value according to expected header length of all lower layer protocols, so that
// allocation of a new page is not necessary
// depends on the used protocols, a good start is a quarter of the page
#ifndef DECOM_MSG_POOL_PAGE_BEGIN
#define DECOM_MSG_POOL_PAGE_BEGIN   (DECOM_MSG_POOL_PAGE_SIZE / 4U)
#endif

///////////////////////////////////////////////////////////////////////////////

namespace decom {

typedef struct tag_pool_page_type
{
  std::uint8_t data[DECOM_MSG_POOL_PAGE_SIZE];  // data buffer
  std::size_t  head;                            // head (begin) position
  std::size_t  tail;                            // tail (end) position (tail == MSG_POOL_PAGE_SIZE: page is full)
  std::size_t  ref;                             // reference counter, 0 = unused page
  bool         read_only;                       // read only flag, if set the content of the page is fixed
  struct tag_pool_page_type* next;              // pointer to the next page
} pool_page_type;



/**
 * message pool class
 */
class msg_pool
{
public:

  typedef pool_page_type      value_type;
  typedef value_type*         pointer;
  typedef const value_type*   const_pointer;
  typedef value_type&         reference;
  typedef const value_type&   const_reference;
  typedef std::size_t         size_type;


  // ctor
  msg_pool()
    : name_("msg_pool")
  {
    // init all pages
    for (size_type i = 0U; i < DECOM_MSG_POOL_PAGES; i++) {
      instance_[i].ref = 0U;
    }
    // init used page counters
    used_pages_     = 0U;   // currently used pages
    used_pages_max_ = 0U;
  }


  // dtor
  ~msg_pool()
  {
    if (used_pages_) {
      DECOM_LOG_ERROR("Allocated pages left back in pool");
    }
  }


  // instance
  pool_page_type instance_[DECOM_MSG_POOL_PAGES];


  // page access
  inline reference operator[](const size_type i)             { return instance_[i]; }
  inline const_reference operator[](const size_type i) const { return instance_[i]; }
  inline reference at(const size_type i)                     { return instance_[i]; }
  inline const_reference at(const size_type i) const         { return instance_[i]; }


  // alloc new page
  pointer page_alloc()
  {
    // lock access
    std::lock_guard<std::mutex> lock(m_);

    // find free page
    for (size_type i = 0U; i < DECOM_MSG_POOL_PAGES; i++) {
      if (!instance_[i].ref) {
        // found it - init page
        instance_[i].ref  = 1U;
        instance_[i].head = 0U;
        instance_[i].tail = 0U;
        instance_[i].next = nullptr;
        if (++used_pages_ > used_pages_max_) {
          used_pages_max_++;
        }
        DECOM_LOG_DEBUG("Page alloc: ") << i << " (" << used_pages_ << "/" << DECOM_MSG_POOL_PAGES << ")";
        return &instance_[i];
      }
    }
    // no free page found
    DECOM_LOG_CRIT("Page allocation failed");
    return nullptr;
  }


  // free page
  inline void page_free(pointer page)
  {
    // lock access
    std::lock_guard<std::mutex> lock(m_);

    if (!--page->ref) {
      used_pages_--;
      DECOM_LOG_DEBUG("Page freed: " << page - instance_ << " (" << used_pages_ << "/" << DECOM_MSG_POOL_PAGES << ")");
    }
  }


  // capacity
  inline size_type used_pages() const     { return used_pages_; }
  inline size_type used_pages_max() const { return used_pages_max_; }
  inline size_type max_size() const       { return DECOM_MSG_POOL_PAGES; }
  inline void clear_used_pages_max()      { used_pages_max_ = 0U; }


  // name
  const char* name_;            // debugging/log name

private:
  size_type used_pages_;        // actual (currently) pages in use
  size_type used_pages_max_;    // maximum of pages used
  std::mutex m_;                // mutex for page pool access
};



/**
 * message iterator class
 */
class msg_iterator : public std::iterator<std::forward_iterator_tag, std::uint8_t>
{
public:
  typedef std::uint8_t        value_type;
  typedef value_type*         pointer;
  typedef const value_type*   const_pointer;
  typedef value_type&         reference;
  typedef const reference     const_reference;
  typedef std::size_t         size_type;


public:
  msg_iterator(msg_pool::pointer page, bool begin)
   : first_(page)
   , page_ (begin ? page       : nullptr)
   , idx_  (begin ? page->head : 0)
  { }


  msg_iterator(const msg_iterator& it)
   : first_(it.first_)
   , page_ (it.page_)
   , idx_  (it.idx_)
  { }


  inline reference operator*() const
  { return page_->data[idx_]; }


  const msg_iterator& operator++()
  {
    DECOM_LOG_ASSERT(page_);
    if (++idx_ >= page_->tail) {
      page_ = page_->next;
      idx_  = page_ ? page_->head : 0U;
    }
    return *this;
  }


  const msg_iterator& operator--()
  {
    // end?
    if (!page_) {
      for (page_ = first_; page_->next; page_ = page_->next);
      idx_ = page_->tail;
    }

    if (idx_ > page_->head) {
      idx_--;
    }
    else {
      // find previous page
      if (page_ != first_) {
        msg_pool::pointer p = first_;
        for (; p->next != page_; p = p->next);
        page_ = p;
        idx_  = p->tail;
      }
    }
    return *this;
  }


  msg_iterator& operator+=(difference_type n)
  {
    if (n > 0) {
      for (; n > 0; n--) {
        ++*this;
      }
    }
    else {
      for (; n < 0; n++) {
        --*this;
      }
    }
    return *this;
  }


  inline msg_iterator& operator-=(difference_type n)
  { return *this += -n; }


  inline msg_iterator operator+(difference_type n) const
  {
    msg_iterator tmp = *this;
    return tmp += n;
  }


  inline msg_iterator operator-(difference_type n) const
  {
    msg_iterator tmp = *this;
    return tmp -= n;
  }


  inline bool operator==(const msg_iterator& other) const
  { return (this->idx_ == other.idx_) && (this->page_ == other.page_); }


  inline bool operator!=(const msg_iterator& other) const
  { return !(*this == other); }


private:
  msg_pool::pointer first_;   // page_ pointer to first page of msg
  msg_pool::pointer page_;    // the actual iterator page of msg
  std::size_t       idx_;     // the actual iterator index in the page
};



/**
 * message class
 */
class msg
{
public:
  typedef std::uint8_t        value_type;
  typedef value_type*         pointer;
  typedef const value_type*   const_pointer;
  typedef value_type&         reference;
  typedef const reference     const_reference;
  typedef msg_iterator        iterator;
  typedef msg_iterator        const_iterator;
  typedef std::size_t         size_type;


  // ctor
  explicit msg(size_type offset = DECOM_MSG_POOL_PAGE_BEGIN)
    : illegal_ref_(0xCCU)                   // init illegal ref
    , name_("msg")
  {
    page_ = get_msg_pool().page_alloc();    // allocate new initial page out of pool
    if (page_) {
      page_->head = page_->tail = offset;   // init pointers
    }
  }


  // ctor with element init
  explicit msg(size_type n, const value_type& value = value_type(), size_type offset = DECOM_MSG_POOL_PAGE_BEGIN)
    : illegal_ref_(0xCCU)                   // init illegal ref
    , name_("msg")
  {
    page_ = get_msg_pool().page_alloc();    // allocate new initial page out of pool
    if (page_) {
      page_->head = page_->tail = offset;   // init pointers
    }
    while (n--) {
      push_back(value);
    }
  }


  // ctor with element copies
  template<typename InputIterator>
  msg(InputIterator first, InputIterator last, size_type offset = DECOM_MSG_POOL_PAGE_BEGIN)
   : illegal_ref_(0xCCU)                  // init illegal ref
   , name_("msg")
  {
    page_ = get_msg_pool().page_alloc();  // allocate new initial page out of pool
    if (page_) {
      page_->head = page_->tail = offset; // init pointers
    }
    for (InputIterator it = first; it != last; ++it) {
      push_back(static_cast<value_type>(*it));
    }
  }


  // copy ctor - This ctor generates a deep (full) copy of the given msg, like
  // the assignment operator
  msg(const msg& m)
    : illegal_ref_(0xCCU)                 // init illegal ref
    , name_("msg")
  {
    page_ = get_msg_pool().page_alloc();  // allocate new initial page out of pool
    for (msg_pool::pointer p = m.page_; p; p = p->next) {
      page_->head = m.page_->head;
      page_->tail = m.page_->tail;
      (void)memcpy(page_->data + page_->head, m.page_->data + page_->head, page_->tail - page_->head);
      if (p->next) {
        page_->next = get_msg_pool().page_alloc();
        if (page_->next) {
          // success
          page_ = page_->next;
        }
        else {
          // page allocation error
        }
      }
    }
  }


  // dtor
  ~msg()
  {
    // message goes out of scope, free all associated pages
    for (msg_pool::pointer p = page_; p; p = p->next) {
      get_msg_pool().page_free(p);
    }
  }


  // assignment operator, make a real copy (physical)
  msg& operator=(const msg& m)
  {
    clear();    // free old pages
    for (msg_pool::pointer p = m.page_; p; p = p->next) {
      page_->head = m.page_->head;
      page_->tail = m.page_->tail;
      (void)memcpy(page_->data + page_->head, m.page_->data + page_->head, page_->tail - page_->head);
      if (p->next) {
        page_->next = get_msg_pool().page_alloc();
        if (page_->next) {
          // success
          page_ = page_->next;
        }
        else {
          // page allocation error
          DECOM_LOG_WARN("assignment error, no free page");
        }
      }
    }
    return *this;
  }


  // Generates a cheap copy (ref copy), not a physical second msg.
  // The ref counters of all pages are incremented. Both messages
  // (original and copy) are read only then!
  msg& ref_copy(const msg& m)
  {
    // free old pages
    for (msg_pool::pointer p = page_; p; p = p->next) {
      get_msg_pool().page_free(p);
    }

    // attach the pages to be copied
    page_ = m.page_;

    // inc refs of new pages
    for (msg_pool::pointer p = page_; p; p = p->next) {
      p->ref++;
    }
    return *this;
  }


  // comparison operator
  bool operator==(const msg& m) const
  {
    // check size
    if (m.size() != size()) {
      return false;
    }
    // check elements
    for (iterator itl = m.begin(), itr = begin(), e = m.end(); itl != e; ++itl, ++itr) {
      if (*itl != *itr) {
        return false;
      }
    }
    return true;
  }


  // comparison operator negative
  inline bool operator!=(const msg& m) const
  { return !(m == *this); }


  // element access
  reference at(size_type pos)
  {
    // security check
    if (!page_) return illegal_ref_;

    // find the according page
    size_type _size = 0U;
    for (msg_pool::pointer p = page_; p; p = p->next) {
      if (pos < _size + (p->tail - p->head)) {
        // page found
        return p->data[p->head + pos - _size];
      }
      _size += p->tail - p->head;
    }
    // page not found (i is out of range)
    DECOM_LOG_WARN("at() position " << pos << " is out of range");
    return illegal_ref_;
  }


  // element access
  inline const_reference at(size_type n) const { return at(n); }
  inline reference operator[](size_type n) { return at(n); }
  inline const_reference operator[](size_type n) const { return at(n); }
  inline reference front() { return *begin(); }
  inline const_reference front() const { return *begin(); }
  inline reference back() { return at(size() - 1U); }
  inline const_reference back() const { return at(size() - 1U); }


  // modifier
  bool push_back(value_type x)
  {
    // security check
    if (!page_ || page_->ref > 1U) {
      DECOM_LOG_WARN("push_back() - " << (!page_ ? "page invalid" : "pageref > 1"));
      return false;
    }
    // store data
    msg_pool::pointer page = last_page();
    if (page->tail == DECOM_MSG_POOL_PAGE_SIZE) {
      // last page is full - allocate a new one
      page->next = get_msg_pool().page_alloc();
      if (page->next) {
        // success
        page = page->next;
      }
      else {
        // page allocation error
        return false;
      }
    }
    // store data
    page->data[page->tail++] = x;
    return true;
  }


  // modifier
  void pop_back()
  {
    // security check
    if (!page_ || page_->ref > 1U) {
      DECOM_LOG_WARN("pop_back() - " << (!page_ ? "page invalid" : "pageref > 1"));
      return;
    }
    // decrement size
    msg_pool::pointer _last_page = last_page();
    if ((--_last_page->tail == 0U) && (page_->next)) {
      // remove page if it's not the only one of this msg
      get_msg_pool().page_free(_last_page);
      msg_pool::pointer p = page_;
      for (; p && p->next && p->next != _last_page; p = p->next);
      p->next = nullptr;
    }
  }


  // modifier
  bool push_front(value_type x)
  {
    // security check
    if (!page_ || page_->ref > 1) {
      DECOM_LOG_WARN("push_front() - " << (!page_ ? "page invalid" : "pageref > 1"));
      return false;
    }
    // store data
    if (page_->head == 0U) {
      // front page is full - allocate a new one
      msg_pool::pointer p = get_msg_pool().page_alloc();
      if (p) {
        // success
        p->head = DECOM_MSG_POOL_PAGE_SIZE;
        p->tail = DECOM_MSG_POOL_PAGE_SIZE;
        p->next = page_;
        page_   = p;
      }
      else {
        // page allocation error
        return false;
      }
    }
    // store data
    page_->data[--page_->head] = x;
    return true;
  }


  // modifier
  void pop_front()
  {
    // security checks
    if (!page_ || page_->ref > 1U || page_->head == page_->tail) {
      DECOM_LOG_WARN("pop_front() - " << (!page_ ? "page invalid" : "pageref > 1"));
      return;
    }
    // increment start position
    if ((++page_->head == DECOM_MSG_POOL_PAGE_SIZE) && (page_->next)) {
      // remove page if it's not the last one
      msg_pool::pointer p = page_;
      page_ = page_->next;
      get_msg_pool().page_free(p);
    }
  }


  // modifier
  void clear()
  {
    if (!page_) { return; };

    for (msg_pool::pointer p = page_; p; p = p->next) {
      get_msg_pool().page_free(p);                            // free page or decrement reference
    }
    page_ = get_msg_pool().page_alloc();                      // allocate new page out of pool
    if (page_) {
      page_->head = page_->tail = DECOM_MSG_POOL_PAGE_BEGIN;  // init pointers
    }
  }


  // insert
  iterator insert(iterator position, const value_type& x)
  {
    if (!page_ || page_->ref > 1U) {
      DECOM_LOG_WARN("insert() - " << (!page_ ? "page invalid" : "pageref > 1"));
      return end();
    }

    push_back(size() ? back() : 0U);  // add new last element
    if (position == end()) {
      position = end() - 1U;
    }
    else {
      for (iterator it = end() - 1U; it != position; --it) {
        *it = *(it - 1U);
      }
    }
    *position = x;

    return position;
  }


  // insert
  inline void insert(iterator position, size_type n, const value_type& x)
  {
    for (; n; n--) {
      insert(position, x);
    }
  }


  // insert
  template<class InputIterator>
  inline void insert(iterator position, InputIterator first, InputIterator last)
  {
    for (InputIterator it = first; it != last; ++it) {
      position = insert(position, *it) + 1;
    }
  }


  // erase
  iterator erase(iterator position)
  {
    if (!page_ || page_->ref > 1U) {
      DECOM_LOG_WARN("erase() - " << (!page_ ? "page invalid" : "pageref > 1"));
      return end();
    }

    if (position == end()) { return end(); }

    for (iterator it = position; it + 1U != end(); ++it) {
      *it = *(it + 1U);
    }
    pop_back();

    return position;
  }


  // erase
  iterator erase(iterator first, iterator last)
  {
    if (first != last) {
      for (iterator::difference_type d = std::distance(first, last); d; d--) {
        first = erase(first);
      }
    }
    return first;
  }


  // iterators
        iterator begin()       { return iterator(page_, true ); }
  const_iterator begin() const { return iterator(page_, true ); }
        iterator end()         { return iterator(page_, false); }
  const_iterator end() const   { return iterator(page_, false); }


  // capacity - size
  // actual size of this message
  inline size_type size() const
  {
    size_type _size = 0U;
    for (msg_pool::pointer p = page_; p; p = p->next) {
      _size += p->tail - p->head;
    }
    return _size;
  }


  // capacity - maximum size
  // maximum possible size of this message
  size_type max_size()
  {
    // return the addition of:
    return size() +                                                                           // actual msg size
      DECOM_MSG_POOL_PAGE_SIZE - last_page()->tail +                                          // rest amount of last page
      (get_msg_pool().max_size() - get_msg_pool().used_pages()) * DECOM_MSG_POOL_PAGE_SIZE;   // size of all free pages in the pool
  }


  // capacity
  // return the size of allocated storage capacity
  size_type capacity() const
  {
    // TBD
    return 0U;
  }


  // capacity
  // true if msg is empty (size == 0)
  inline bool empty() const
  { return size() == 0U; }


  // resize msg to given size
  bool resize(size_type sz)
  {
    // security check
    if (!page_ || page_->ref > 1U) {
      DECOM_LOG_WARN("resize() - " << (!page_ ? "page invalid" : "pageref > 1"));
      return false;
    }

    size_type msg_size = size();
    if (sz == msg_size) {
      // nothing to do
      return true;
    }

    if (sz > msg_size) {
      // grow
      msg_pool::pointer page = last_page();
      for (size_type i = 0U; i < sz - msg_size; i++) {
        if (page->tail == DECOM_MSG_POOL_PAGE_SIZE) {
          // last page is full - allocate a new one
          page->next = get_msg_pool().page_alloc();
          if (page->next) {
            // success
            page = page->next;
          }
          else {
            // page allocation error
            return false;
          }
        }
        page->data[page->tail++] = 0U;  // init data
      }
    }
    else {
      // shrink
      for (size_type i = 0U; i < msg_size - sz; i++) {
        msg_pool::pointer _last_page = last_page();
        if ((--_last_page->tail == 0U) && (page_->next)) {
          // remove page if it's not the only one of this msg
          get_msg_pool().page_free(_last_page);
          msg_pool::pointer p = page_;
          for (; p && p->next && p->next != _last_page; p = p->next);
          p->next = nullptr;
        }
      }
    }
    return true;
  }


  /**
   * Copy msg to a linear byte buffer
   * \param dest Destination buffer
   * \param maxlength Maximum length/size of destination buffer (copied bytes)
   * \param offset Start offset in msg to access message fragments
   * \return Pointer to destination buffer
   */
  void* get(std::uint8_t* dest, size_type maxlength, size_type offset = 0U) const
  {
    // security check
    if (!dest || !maxlength || offset >= size()) {
      return nullptr;
    }

    // copy in page sizes
    size_type offset_page = 0U;   // offset of all previous pages
    size_type offset_dest = 0U;   // offset in destination buffer (copied bytes)
    for (msg_pool::pointer p = page_; p; p = p->next) {
      size_type page_start = p->head;
      size_type page_size = p->tail - p->head;

      // start within this page?
      if ((offset > offset_page) && (offset < offset_page + page_size)) {
        // yes - adjust start position and page_size
        page_start += offset - offset_page;
        page_size -= offset - offset_page;
      }

      if (offset < offset_page + page_size) {
        // check maxsize adjust page_size
        if (offset_dest + page_size > maxlength) {
          page_size = maxlength - offset_dest;
        }
        // copy to dest buffer
        (void)memcpy(dest + offset_dest, &p->data[page_start], page_size);
        offset_dest += page_size;
      }
      offset_page += page_size;

      // stop if maxsize is reached
      if (offset_dest >= maxlength) {
        break;
      }
    }
    return dest;
  }


  /**
   * Copy a linear byte buffer to msg
   * \param source Source buffer
   * \param count Number of elements
   * \return true if successful
   */
  bool put(const std::uint8_t* source, size_type count)
  {
    // security check
    if (!source || !count || count > max_size()) {
      return false;
    }

    // remove old msg data
    clear();

    msg_pool::pointer page = page_;
    size_type offset = 0U;
    // copy data in page sized chunks
    do {
      page->tail = page->tail + count > DECOM_MSG_POOL_PAGE_SIZE ? DECOM_MSG_POOL_PAGE_SIZE : page->tail + count;
      (void)memcpy(page->data + page->head, source + offset, page->tail - page->head);
      count -= page->tail - page->head;
      offset += page->tail - page->head;
      if (count) {
        page->next = get_msg_pool().page_alloc();
        if (page->next) {
          // success
          page = page->next;
        }
        else {
          // page allocation error - clean up whole msg
          clear();
          return false;
        }
      }
    } while (count);
    return true;
  }


  /**
   * Append a second message to this msg
   * \param second Second message
   */
  void append(msg& second)
  {
    // concat all pages to this object
    last_page()->next = second.page_;
    // inc page references of second message
    for (msg_pool::pointer p = second.page_; p; p = p->next) {
      p->ref++;
    }
  }


  /**
   * Module name - required for debugging
   */
  const char* name_;


  /**
   * msg pool access
   */
  static inline msg_pool& get_msg_pool()
  {
    static msg_pool _msg_pool;
    return _msg_pool;
  }


private:
  // return a pointer to the last page of this msg
  inline msg_pool::pointer last_page() const
  {
    msg_pool::pointer p = page_;
    for (; p->next; p = p->next);
    return p;
  }

  value_type illegal_ref_;      // illegal ref, returned if [] or 'at' is out of bounds
  msg_pool::pointer page_;      // first page of the message
};

} // namespace decom

#endif // _DECOM_MSG_H_
