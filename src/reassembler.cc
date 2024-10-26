#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t lim = next_ + writer().available_capacity(), n = data.size();
  if (is_last_substring) {
    len_ = min(len_, first_index + n);
  }
  if (first_index >= lim) {
    return; // unacceptable
  }
  if (first_index + n > lim) {
    n = lim - first_index; // cut off unacceptable part
  }

  for (uint64_t i = max(0l, (int64_t)next_ - (int64_t)first_index); i < n; i++) {
    mp_[first_index + i] = data[i];
  }
  
  while (mp_.size() && mp_.begin()->first == next_) {
    writer().push(string(1, mp_.begin()->second));
    next_++;
    mp_.extract(mp_.begin());
  }

  if (next_ == len_) writer().close();
}

uint64_t Reassembler::bytes_pending() const
{
  return mp_.size();
}
