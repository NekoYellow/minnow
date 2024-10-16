#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {
  q_.reserve( capacity );
}

bool Writer::is_closed() const
{
  return closed_;
}

void Writer::push( string data )
{
  if (is_closed()) {
    // set_error();
    return;
  }
  uint64_t len = min(data.size(), available_capacity());
  if (len == 0) return;
  data.resize(len);
  copy(begin(data), end(data), back_inserter(q_));
  tot_pushed_ += len;
}

void Writer::close()
{
  closed_ = true;
}

uint64_t Writer::available_capacity() const
{
  return capacity_ - q_.size();
}

uint64_t Writer::bytes_pushed() const
{
  return tot_pushed_;
}

bool Reader::is_finished() const
{
  return writer().is_closed() && bytes_buffered() == 0;
}

uint64_t Reader::bytes_popped() const
{
  return tot_popped_;
}

string_view Reader::peek() const
{
  return {q_.data(), q_.size()};
}

void Reader::pop( uint64_t len )
{
  len = min(len, bytes_buffered());
  q_.erase(begin(q_), begin(q_) + len);
  tot_popped_ += len;
}

uint64_t Reader::bytes_buffered() const
{
  return q_.size();
}
