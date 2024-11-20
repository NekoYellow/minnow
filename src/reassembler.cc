#include "reassembler.hh"
#include <algorithm>

using namespace std;

void Reassembler::insert( uint64_t pos, string data, bool is_last_substring )
{
  if ( is_last_substring ) {
    eof_ = min( eof_, pos + data.size() );
  }

  const uint64_t lim = next_ + writer().available_capacity();
  if ( pos >= lim ) {
    return; // unacceptable
  }
  if ( pos + data.size() >= lim ) {
    data.resize( lim - pos ); // cut off unacceptable part
  }

  if ( pos <= next_ ) {
    emit( pos, move( data ) );
  } else {
    cache( pos, move( data ) );
  }

  for ( ; buffer_.size(); buffer_.pop_front() ) {
    auto&& [idx, s] = buffer_.front();
    if ( idx > next_ ) {
      break;
    }
    num_bytes_ -= s.size();
    emit( idx, move( s ) );
  }
}

uint64_t Reassembler::bytes_pending() const
{
  return num_bytes_;
}

void Reassembler::emit( uint64_t pos, string data )
{
  if ( pos < next_ ) {
    data.erase( 0, next_ - pos ); // cut off duplicate part
  }
  next_ += data.size();
  output_.writer().push( move( data ) );

  if ( next_ >= eof_ ) {
    output_.writer().close();
    num_bytes_ = 0;
  }
}

void Reassembler::cache( uint64_t pos, string data )
{
  auto end = buffer_.end();
  auto left = lower_bound( buffer_.begin(), end, pos, []( auto&& e, uint64_t idx ) -> bool {
    return idx > e.first + e.second.size();
  } );
  auto right = upper_bound( left, end, pos + data.size(), []( uint64_t nxt_idx, auto&& e ) -> bool {
    return nxt_idx < e.first;
  } );

  uint64_t next_pos = pos + data.size();
  if ( left != end ) {
    auto&& [l_curr, s] = *left;
    uint64_t r_curr = l_curr + s.size();
    if ( pos >= l_curr && next_pos <= r_curr )
      return;
    else if ( next_pos < l_curr ) {
      right = left;
    } else if ( !( pos <= l_curr && r_curr <= next_pos ) ) {
      if ( pos >= l_curr ) {
        data.insert( 0, string( s.begin(), s.begin() + s.size() - ( r_curr - pos ) ) );
      } else {
        data.resize( data.size() - ( next_pos - l_curr ) );
        data.append( s );
      }
      pos = min( pos, l_curr );
    }
  }

  next_pos = pos + data.size();
  if ( right != left && buffer_.size() ) {
    auto&& [l_curr, s] = *prev( right );
    if ( l_curr + s.size() > next_pos ) {
      data.resize( data.size() - ( next_pos - l_curr ) );
      data.append( s );
    }
  }

  for ( ; left != right; left = buffer_.erase( left ) ) {
    num_bytes_ -= left->second.size();
  }
  num_bytes_ += data.size();
  buffer_.insert( left, { pos, move( data ) } );
}
