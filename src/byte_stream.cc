#include "byte_stream.hh"

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

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
  uint64_t len = min(data.size(), available_capacity());
  if (len == 0) return;
  for (uint64_t i = 0; i < len; i++) {
    q_.push(data[i]);
  }
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
  if (!bytes_buffered()) return {};
  return {&q_.front(), 1};
}

void Reader::pop( uint64_t len )
{
  if (len > bytes_buffered()) {
    len = bytes_buffered();
  }
  tot_popped_ += len;
  for (; len; len--) {
    q_.pop();
  }
}

uint64_t Reader::bytes_buffered() const
{
  return q_.size();
}
