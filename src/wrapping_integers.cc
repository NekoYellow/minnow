#include "wrapping_integers.hh"

using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return Wrap32 { zero_point + n };
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  uint64_t n = raw_value_ - zero_point.raw_value_;
  uint64_t m = checkpoint / (1ull << 32);

  uint64_t mnd = UINT64_MAX, opt = n;
  for ( uint64_t i = (!m ? m : m-1), x, d; i <= m+1; i++ ) {
    x = n + i * (1ul << 32);
    d = (x > checkpoint ? x - checkpoint : checkpoint - x);
    if ( d < mnd ) mnd = d, opt = x;
  }
  return opt;
}
