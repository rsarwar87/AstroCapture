
#ifndef __CIRCULAR__
#define __CIRCULAR__
//-------------------------------------------------------------------
// Includes
//-------------------------------------------------------------------
#include <spdlog/spdlog.h>

#include <memory>
#include <atomic>

//-------------------------------------------------------------------
template <class T>
class Circular_Buffer {
 private:
  //---------------------------------------------------------------
  // Circular_Buffer - Private Member Variables
  //---------------------------------------------------------------

  std::unique_ptr<T[]> buffer;  // using a smart pointer is safer (and we don't
                                // have to implement a destructor)
  std::atomic<std::size_t>  head = 0;              // size_t is an unsigned long
  std::atomic<std::size_t>  tail = 0;
  std::atomic<std::size_t>  m_ltail = 0;
  const size_t max_size;
  const size_t n_bytes;
  bool is_done = true;
  T* empty_item;  // we will use this to clear data
 public:
  //---------------------------------------------------------------
  // Circular_Buffer - Public Methods
  //---------------------------------------------------------------

  // Create a new Circular_Buffer.
  Circular_Buffer<T>(size_t _nsamples, size_t _nbytes)
      : buffer(std::unique_ptr<T[]>(new T[_nsamples * _nbytes])),
        max_size(_nsamples),
        n_bytes(_nbytes) {
    empty_item = new T[_nsamples * _nbytes];
    spdlog::info(
        "Streaming buffer created of size {} bytes * {} samples = {} MB",
        _nbytes, _nsamples, _nsamples * _nbytes / 1024 / 1024);
  };

  ~Circular_Buffer<T>() { spdlog::info("Streaming buffer released"); }
  T get_empty() { return *empty_item; }

  // Add an item to this circular buffer.
  T* get_new_buffer() {
    // if buffer is full, throw an error
    if (is_full()) {
      spdlog::critical("Streaming buffer is full");
      return nullptr;
    }

    // insert item at back of buffer
    T* item = &buffer[tail * n_bytes];
    size_t tt = tail;
    m_ltail = tt;

    // increment tail
    tail = (tail + 1) % max_size;
    return item;
  }

  // Remove an item from this circular buffer and return it.
  T* dequeue() {
    // if buffer is empty, throw an error
    if (is_empty()) {
      spdlog::critical("buffer is empty");
      return empty_item;
    }
    // get item at head
    T* item = &buffer[head * n_bytes];

    // set item at head to be empty
    // T empty;
    // buffer[head] = empty_item;

    // move head foward
    is_done = false;
    //head = (head + 1) % max_size;

    // return item
    return item;
  }
  void move_trail()
  {
    if (!is_done)
    {
      is_done = true;
      head = (head + 1) % max_size;
    }
  }

  // Return the item at the front/end of this circular buffer.
  T* peek() { return &buffer[head * n_bytes]; }
  T* last() { return &buffer[m_ltail * n_bytes]; }

  // Return true if this circular buffer is empty, and false otherwise.
  bool is_empty() { return occupancy() == 0; }

  // Return true if this circular buffer is full, and false otherwise.
  bool is_full() { return  occupancy() == max_size - 1;}

  // Return the size of this circular buffer.
  std::array<size_t,2> get_head_tail() {
    return std::array<size_t,2> {head, tail};
  }
  // Return the number of empty slots on the buffer.
  size_t vacancy() {
    return max_size - occupancy();
  }
  // Return the number of populated slots on the buffer.
  size_t occupancy() {
    if (tail >= head) return tail - head;
    return max_size - head + tail;
  }
  float update_fullness()
  {
    return float(occupancy())/float(max_size);
  }
};

#endif
