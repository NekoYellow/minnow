#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  uint64_t lim = next_ + writer().available_capacity();
  uint64_t l = first_index, r = first_index + data.size();
  if (is_last_substring) {
    len_ = min(len_, r);
  }
  if (l >= lim) {
    return; // unacceptable
  }
  r = min(r, lim); // cut off unacceptable part

  uint64_t start = max(next_, l);
  for (uint64_t i = start; i < r; i++) {
    auto p = seg_.lower_bound(i);
    if (p != seg_.end() && p->first == i) {
      i = p->second;
    } else if (p != seg_.begin()) {
      auto [pl, pr] = *(--p);
      i = max(i, pr);
    }
    if (i >= r) break;
    mp_[i] = data[i - l];
  }

  while (true) {
    auto p = seg_.upper_bound(start);
    if (p == seg_.end()) break;
    auto [pl, pr] = *p;
    if (pr <= r) seg_.extract(p);
  }

  auto p = seg_.lower_bound(start);
  if (p != seg_.end() && p->first == start) {
    seg_[start] = max(p->second, r);
  } else if (p != seg_.begin()) {
    auto [pl, pr] = *(--p);
    if (pr >= start) seg_[pl] = max(pr, r);
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
