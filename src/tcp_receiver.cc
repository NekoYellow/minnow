#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.RST ) {
    reassembler_.reader().set_error();
    return;
  }

  const uint64_t checkpoint = reassembler_.writer().bytes_pushed() + ISN_.has_value();
  if ( !ISN_.has_value() ) {
    if ( message.SYN ) {
      ISN_ = message.seqno;
    } else {
      return;
    }
  }

  if ( message.seqno == ISN_ && checkpoint > 0 ) {
    return;
  }

  const uint64_t absolute_seqno = message.seqno.unwrap( *ISN_, checkpoint );
  const uint64_t insertion_point = absolute_seqno > 0 ? absolute_seqno - 1 : 0;
  reassembler_.insert( insertion_point, move( message.payload ), message.FIN );
}

TCPReceiverMessage TCPReceiver::send() const
{
  const uint64_t checkpoint = reassembler_.writer().bytes_pushed() + ISN_.has_value();
  const uint64_t available_capacity = reassembler_.writer().available_capacity();

  const uint16_t window_size
    = static_cast<uint16_t>( min( available_capacity, static_cast<uint64_t>( UINT16_MAX ) ) );

  if ( !ISN_.has_value() ) {
    return { {}, window_size, reassembler_.writer().has_error() };
  }

  const bool is_closed = reassembler_.writer().is_closed();
  const uint64_t ack_seqno = checkpoint + is_closed;
  return { Wrap32::wrap( ack_seqno, *ISN_ ), window_size, reassembler_.writer().has_error() };
}