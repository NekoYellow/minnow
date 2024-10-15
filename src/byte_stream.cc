#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ), s_(' ', capacity), l_(0), r_(0) {}

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::push( string data )
{
  if (is_closed()) {
    set_error();
    return;
  }
  uint64_t n = data.size();
  if (n > available_capacity()) {
    set_error();
    return;
  }
  for (uint64_t i = 0; i < n; i++) {
    s_[r_++] = data[i];
    if (r_ == capacity_) r_ = 0;
  }
  n_ += n;
  tot_ += n;
}

void Writer::close()
{
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - n_;
}

uint64_t Writer::bytes_pushed() const
{
  return tot_;
}

bool Reader::is_finished() const
{
  // Your code here.
  return {};
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return {};
}

string_view Reader::peek() const
{
  // Your code here.
  return {};
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  (void)len;
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return {};
}
