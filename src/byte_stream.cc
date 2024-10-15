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
  tot_pushed_ += n;
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
  return tot_pushed_;
}

bool Reader::is_finished() const
{
  return closed_ && !n_;
}

uint64_t Reader::bytes_popped() const
{
  return tot_popped_;
}

string_view Reader::peek() const
{
  if (!n_) return {};
  else if (l_ < r_) return {s_.begin() + l_, s_.begin() + r_};
  else return string{s_.begin() + l_, s_.end()} + string{s_.begin(), s_.begin() + r_};
}

void Reader::pop( uint64_t len )
{
  if (len > n_) {
    set_error();
    return;
  }
  for (uint64_t i = 0; i < len; i++) {
    l_++;
    if (l_ == capacity_) l_ = 0;
  }
  n_ -= len;
  tot_popped_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  return n_;
}
