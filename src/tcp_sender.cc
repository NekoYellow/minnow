#include "tcp_sender.hh"
#include "tcp_config.hh"

using namespace std;

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  return num_seqno_in_flight_;
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  return cons_retrans_;
}

void TCPSender::push( const TransmitFunction& transmit )
{
  // If the window is full, no new data can be sent
  if ( ( window_size_ && num_seqno_in_flight_ >= window_size_ )
       || ( window_size_ == 0 && num_seqno_in_flight_ >= 1 ) ) {
    return;
  }

  // Compute the initial sequence number for the next segment
  auto seqno = Wrap32::wrap( ackno_, isn_ );

  // Compute available window size
  uint64_t win
    = window_size_ == 0 ? 1 : window_size_ - num_seqno_in_flight_ - ( seqno == isn_ );

  string out;
  // Read data from the stream into the outgoing buffer
  while ( reader().bytes_buffered() and out.size() < win ) {
    auto view = reader().peek();

    view = view.substr( 0, win - out.size() ); // Limit to the available window size
    out += view;
    input_.reader().pop( view.size() );
  }

  size_t len;
  string_view view( out );

  // Create and transmit TCP segments until all data is sent
  while ( !view.empty() || seqno == isn_ || ( !finished_ && writer().is_closed() ) ) {
    len = min( view.size(), TCPConfig::MAX_PAYLOAD_SIZE );

    string payload( view.substr( 0, len ) );

    // Construct a TCP segment
    TCPSenderMessage message { seqno, seqno == isn_, move( payload ), false, writer().has_error() };

    // Append a FIN flag if all data has been sent and the stream is closed
    if ( !finished_ && writer().is_closed() && len == view.size()
         && ( num_seqno_in_flight_ + message.sequence_length() < window_size_
              || ( window_size_ == 0 && message.sequence_length() == 0 ) ) ) {
      finished_ = message.FIN = true;
    }

    transmit( message );

    // Update sender state
    ackno_ += message.sequence_length();
    num_seqno_in_flight_ += message.sequence_length();
    msg_q_.emplace( move( message ) );

    // Break if no more data needs to be sent
    if ( !finished_ && writer().is_closed() && len == view.size() ) {
      break;
    }

    seqno = Wrap32::wrap( ackno_, isn_ );
    view.remove_prefix( len );
  }
}

TCPSenderMessage TCPSender::make_empty_message() const
{
  // Create an empty TCP segment (used for ACKs or keep-alives)
  return { Wrap32::wrap( ackno_, isn_ ), false, "", false, writer().has_error() };
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  if ( msg.RST ) {
    writer().set_error(); // Handle reset (RST) message
    return;
  }

  // Treat a '0' window size as equal to '1' but don't back off RTO
  window_size_ = msg.window_size;
  uint64_t ackno_rcv = msg.ackno ? msg.ackno.value().unwrap( isn_, ackno_old_ ) : 0;

  // Update sender state if acknowledgment advances
  if ( ackno_old_ < ackno_rcv && ackno_rcv <= ackno_ ) {
    ackno_old_ = ackno_rcv;

    timer_ms_ = 0; // Reset retransmission timer
    RTO_ms_ = initial_RTO_ms_; // Reset retransmission timeout
    cons_retrans_ = 0;

    // Remove acknowledged segments from the queue
    uint64_t i = 0;
    while ( !msg_q_.empty() && i <= ackno_rcv ) {
      i = msg_q_.front().seqno.unwrap( isn_, ackno_old_ ) + msg_q_.front().sequence_length();
      if ( i <= ackno_rcv ) {
        num_seqno_in_flight_ -= msg_q_.front().sequence_length();
        msg_q_.pop();
      }
    }
  }
}

void TCPSender::tick( uint64_t ms_since_last_tick, const TransmitFunction& transmit )
{
  // Only update timer if there are unacknowledged segments
  if ( !msg_q_.empty() ) {
    timer_ms_ += ms_since_last_tick;
  }

  // Check if retransmission timeout has occurred
  if ( timer_ms_ >= RTO_ms_ ) {
    transmit( msg_q_.front() ); // Retransmit the first unacknowledged segment
    if ( window_size_ > 0 ) {
      ++cons_retrans_;
      RTO_ms_ <<= 1; // Exponential backoff
    }
    timer_ms_ = 0;
  }
}
