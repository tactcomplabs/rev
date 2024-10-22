// Copyright 2009-2024 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2024, NTESS
// All rights reserved.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

#include "kgdbg.h"
#include "probe.h"

namespace SSTDEBUG::Probe {

ProbeControl::ProbeControl(
  SST::Component* comp,
  SST::Output*    out,
  int             mode,
  int             startCycle,
  int             endCycle,
  int             bufferSize,
  int             port,
  int             postDelay,
  uint64_t        cliControl
)
  : comp_( comp ), out_( out ), probeBufCtl_( nullptr ), mode_( mode ), startCycle_( startCycle ), endCycle_( endCycle ),
    bufferSize_( bufferSize ), port_( port ), postDelayCounter_( postDelay ), cliControl_( cliControl ) {
  // If disabled nothing to do.
  if( !mode_ )
    return;

  // Disable end cycle if illegal
  if( endCycle_ <= startCycle_ )
    endCycle_ = 0;

  out_->verbose( CALL_INFO, 1, 0, "probeMode=%d\n", mode_ );
  out_->verbose( CALL_INFO, 1, 0, "probeStartCycle=%d\n", startCycle_ );
  out_->verbose( CALL_INFO, 1, 0, "probeEndCycle=%d\n", endCycle_ );
  out_->verbose( CALL_INFO, 1, 0, "probeBufferSize=%d\n", bufferSize_ );
  out_->verbose( CALL_INFO, 1, 0, "probePort=%d\n", port_ );
  out_->verbose( CALL_INFO, 1, 0, "probePostDelay=%d\n", postDelayCounter_ );
  out_->verbose( CALL_INFO, 1, 0, "cliControl=%" PRIx64 "\n", cliControl_.v );

  postDelayInitCount_ = postDelayCounter_;
  useDelayCounter_    = ( postDelayCounter_ >= 0 );
  if( !useDelayCounter_ )
    out_->verbose( CALL_INFO, 1, 0, "continue post-trigger sampling\n" );

  if( mode_ != 1 )
    out_->fatal( CALL_INFO, -1, "probeMode>1 is not supported\n" );

  // Note: The probing will start with the first checkpoint.
  // Could consider starting the probing at time 0 but this may be controllable
  // by the checkpoint settings instead.
  syncState_ = SyncState::WAIT;

  kgdbg::spinner( "PROBE_SPINNER" );
}

ProbeControl::~ProbeControl() {}

void ProbeControl::updateSyncState( int cycle ) {
  syncCycle = cycle;
  switch( syncState_ ) {
  case SyncState::WAIT:
    if( cycle >= startCycle_ ) {
      out_->verbose( CALL_INFO, 1, 0, "probe active at cycle %d\n", cycle );
      syncState_ = SyncState::ACTIVE;
    }
    break;
  case SyncState::ACTIVE:
    handleSyncPointActions();
    if( endCycle_ && ( cycle >= endCycle_ ) ) {
      out_->verbose( CALL_INFO, 1, 0, "probe disabled at cycle %d\n", cycle );
      syncState_ = SyncState::INVALID;
    }
    break;
  default: break;
  }

  // Conditionally enter CLI
  bool do_cli = false;
  if( cliControl_.f.chkpt_all ) {
    do_cli = true;
  } else if( cliControl_.f.chkpt_active && ( syncState_ == SyncState::ACTIVE ) ) {
    do_cli = true;
  } else if( cliControl_.f.chkpt_states && ( lastSyncState_ != syncState_ ) ) {
    do_cli = true;
  }
  if( do_cli ) {
    cliControl_.f.chkpt_cli = 1;
    updateCLI();
    cliControl_.f.chkpt_cli = 0;
  }

  lastSyncState_ = syncState_;
}

void ProbeControl::updateProbeState( int cycle ) {
  // TODO the sample and trigger functions may be able to manage the probe state
  switch( probeState_ ) {
  case ProbeState::IDLE:
    if( syncState_ == SyncState::ACTIVE ) {
      // start sampling and wait for trigger to move us post sampling state.
      probeState_ = ProbeState::PRE_SAMPLING;
      out_->verbose( CALL_INFO, 1, 0, "probe in pre-trigger sampling phase\n" );
    }
    break;
  case ProbeState::POST_SAMPLING:
    if( useDelayCounter_ && ( postDelayCounter_ < 0 ) ) {
      // trigger detected. Indicate flush and wait until sync action clears
      out_->verbose( CALL_INFO, 1, 0, "Sampling stopped at cycle %d\n", cycle );
      syncActions_.f.flush2stdout = true;
      probeState_                 = ProbeState::WAIT;
    }
    break;
  default: break;
  }

  // Conditionally enter CLI ( also need to check in the sample() function )
  if( cliControl_.f.probe_states && ( lastProbeState_ != probeState_ ) )
    updateCLI();

  lastProbeState_ = probeState_;  // used for detecting state changes
}

void ProbeControl::handleSyncPointActions() {
  // If we are sampling until checkpoint
  if( ( !useDelayCounter_ ) && ( probeState_ == ProbeState::POST_SAMPLING ) )
    probeState_ = ProbeState::WAIT;

  // Only when probe is in WAIT state is a checkpoint action needed
  if( probeState_ != ProbeState::WAIT )
    return;

  assert( syncState_ == SyncState::ACTIVE );
  assert( probeBufCtl_ );

  if( syncActions_.f.flush2stdout ) {
    out_->verbose( CALL_INFO, 1, 0, "syncAction: flush to stdout\n" );
    syncActions_.f.flush2stdout = 0;
    probeBufCtl_->render_buffer( std::cout );
    probeBufCtl_->reset_buffer();
  }
  if( syncActions_.f.flush2file ) {
    out_->verbose( CALL_INFO, 1, 0, "syncAction: flush to file (TODO)\n" );
    syncActions_.f.flush2file = 0;
    // TODO 1 file per rank
    // probeBufCtl_->render_buffer(ofile);
    probeBufCtl_->reset_buffer();
  }
  // continuation / stop actions
  if( syncActions_.f.shutdown ) {
    out_->verbose( CALL_INFO, 1, 0, "syncAction: shutdown\n" );
    syncActions_.v = 0;  // clear all unhandled actions
    syncState_     = SyncState::IDLE;
    probeState_    = ProbeState::IDLE;
    // TODO Is there a way to signal a normal shutdown?
    out_->fatal( CALL_INFO, -1, "-- NORMAL == Probe initiated shutdown" );
  } else if( syncActions_.f.cont ) {
    out_->verbose( CALL_INFO, 1, 0, "syncAction: cont\n" );
    syncActions_.f.cont   = 0;
    syncActions_.f.repeat = 0;  // clear lower priority actions
    postDelayCounter_     = postDelayInitCount_;
    probeState_           = ProbeState::POST_SAMPLING;
  } else if( syncActions_.f.repeat ) {
    out_->verbose( CALL_INFO, 1, 0, "syncAction: repeat\n" );
    syncActions_.f.repeat = 0;
    probeBufCtl_->reset_trigger();
    probeBufCtl_->reset_buffer();
    postDelayCounter_ = postDelayInitCount_;  // TODO move counter into bufferControl?
    probeState_       = ProbeState::PRE_SAMPLING;
  } else {
    // default: stop probing and let simulation run free
    out_->verbose( CALL_INFO, 1, 0, "syncAction: exit probe\n" );
    syncState_  = SyncState::IDLE;
    probeState_ = ProbeState::IDLE;
  }
  // All actions should have been handled and cleared.
  assert( syncActions_.v == 0 );
}

void ProbeControl::sample() {
  if( useDelayCounter_ && ( probeState_ == ProbeState::POST_SAMPLING ) )
    postDelayCounter_--;

  // conditionally enter CLI
  // State change check in updateProbeState())
  bool do_cli = false;
  if( cliControl_.f.probe_samples )
    do_cli = true;
  else if( cliControl_.f.probe_trig2samples && ( probeState_ == ProbeState::POST_SAMPLING ) )
    do_cli = true;
  if( do_cli )
    updateCLI();
}

void ProbeControl::trigger( bool cond ) {
  if( probeState_ != ProbeState::PRE_SAMPLING )
    return;
  if( cond ) {
    out_->verbose( CALL_INFO, 1, 0, "Detected Trigger\n" );
    probeState_ = ProbeState::POST_SAMPLING;
    probeBufCtl_->markAsTriggerRec();
    if( !useDelayCounter_ )
      out_->verbose( CALL_INFO, 1, 0, "Continue sampling\n" );
    else if( postDelayCounter_ )
      out_->verbose( CALL_INFO, 1, 0, "Sampling for %d additional samples\n", postDelayCounter_ );
  }
}

void ProbeControl::setBufferControls( std::shared_ptr<ProbeBufCtl> probeBufCtl ) {
  probeBufCtl_ = probeBufCtl;
}

void ProbeControl::updateCLI() {
  if( port_ == 0 )
    return;

  // start up server if needed.
  if( probeSocket_ == nullptr ) {
    probeSocket_ = std::make_unique<ProbeSocket>( port_, this, this->comp_, out_ );
    if( probeSocket_->create() != ProbeSocket::RESULT::SUCCESS )
      out_->fatal( CALL_INFO, -1, "Could not create debug port %d\n", port_ );
    if( probeSocket_->connect() != ProbeSocket::RESULT::SUCCESS )
      out_->fatal( CALL_INFO, -1, "Could not connect to debug port %d\n", port_ );
  }
  // If connected run the CLI handler
  if( probeSocket_->connected() ) {
    if( probeSocket_->cli_handler() != ProbeSocket::RESULT::SUCCESS )
      out_->fatal( CALL_INFO, -1, "An error occured on debug port %d\n", port_ );
  }
}

ProbeBufCtl::ProbeBufCtl( int sz ) : sz_( sz ) {
  tags.resize( sz );
};

void ProbeBufCtl::reset_buffer() {
  num_recs     = 0;
  cur          = 0;
  first        = 0;
  samples_lost = 0;
}

void ProbeBufCtl::reset_trigger() {
  state = CLEAR;
}

void ProbeBufCtl::markAsTriggerRec() {
  state = TRIGREC;
}

void ProbeBufCtl::capture() {
  // child class calls this first to establish pointer state before saving record
  // When this buffer fills it writes over the oldest data

  if( num_recs == 0 ) {
    // empty
    num_recs++;
    first = 0;
    cur   = 0;
  } else if( num_recs < sz_ ) {
    num_recs++;
    cur = ( cur + 1 ) % sz_;
    if( cur == 0 )
      first = 1;
  } else {
    // full
    cur   = first;
    first = ( first + 1 ) % sz_;
  }

  //std::cout << "state=" << getTrigStateChar() <<  " sz=" << sz_ << " num_recs=" << num_recs << " first=" << first << " cur=" << cur << std::endl;

  if( state == OVERRUN )
    samples_lost++;

  // Check tags before overwriting them
  if( ( state == TRIGGERED ) && ( tags.at( cur ) == TRIGREC ) ) {
    //std::cout << "overrun!" << std::endl;
    state     = OVERRUN;
    tags[cur] = OVERRUN;  // override tag
  } else if( state == TRIGREC ) {
    tags[cur] = state;
    state     = TRIGGERED;
  } else {
    tags[cur] = state;
  }
}

void ProbeBufCtl::render_buffer( std::ostream& os ) {
  // os << "rendering buffer" << std::endl;
  // os << "trigger state: " << state << std::endl;
  // TODO limit range
  os << "#I " << num_recs << " records" << std::endl;
  if( num_recs == 0 )
    return;
  if( state == OVERRUN ) {
    os << "#I saved trigger due to buffer overflow\n";
    os << "#";
    render_trigger_rec( os, 'T' );
    os << std::endl;
    if( samples_lost > 0 )
      os << "# records discarded " << samples_lost << std::endl;
  }
  for( int i = 0; i < num_recs; i++ ) {
    os << "#";
    int  idx = ( first + i ) % sz_;
    char pfx = ( ( state == TRIGGERED ) && ( tags.at( idx ) == TRIGREC ) ) ? 'T' : ' ';
    render( os, idx, pfx );
    os << std::endl;
  }
}

ProbeSocket::ProbeSocket( int port, ProbeControl* probeControl, SST::Component* comp, SST::Output* out )
  : port_( port ), probeControl_( probeControl ), comp_( comp ), out_( out ) {}

ProbeSocket::~ProbeSocket() {
  if( socket_state_ >= SOCKET_STATE::CREATED ) {
    if( serverSock_ >= 0 )
      close( serverSock_ );
  }
  if( socket_state_ >= SOCKET_STATE::CONNECTED ) {
    if( clientSock_ >= 0 )
      close( clientSock_ );
  }
}

ProbeSocket::RESULT ProbeSocket::create() {
  serverSock_ = socket( AF_INET, SOCK_STREAM, 0 );
  if( serverSock_ < 0 )
    return RESULT::CREATION_ERROR;
  sockaddr_in serv_addr;
  memset( (char*) &serv_addr, 0, sizeof( serv_addr ) );
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port        = htons( port_ );
  int rc                    = bind( serverSock_, (struct sockaddr*) &serv_addr, sizeof( serv_addr ) );
  if( rc < 0 )
    return RESULT::BINDING_ERROR;
  socket_state_ = SOCKET_STATE::CREATED;
  return RESULT::SUCCESS;
}

ProbeSocket::RESULT ProbeSocket::connect() {
  if( socket_state_ != SOCKET_STATE::CREATED )
    return RESULT::INVALID;
  gethostname( buffer_, sizeof( buffer_ ) );
  hostname_ = std::string( buffer_ );
  out_->verbose( CALL_INFO, 1, 0, "Waiting for connection on %s:%d\n", hostname_.c_str(), port_ );
  // block until client connects (limit 1)
  listen( serverSock_, 1 );
  clientSock_ = accept( serverSock_, nullptr, nullptr );
  if( clientSock_ < 0 ) {
    perror( "accept failed: " );
    return RESULT::ACCEPTING_ERROR;
  }
  socket_state_ = SOCKET_STATE::CONNECTED;
  out_->verbose( CALL_INFO, 1, 0, "Connected\n" );  // TODO client identification on connection
  return RESULT::SUCCESS;
}

ProbeSocket::RESULT ProbeSocket::cli_handler() {
  if( socket_state_ == SOCKET_STATE::DISCONNECTING ) {
    close( clientSock_ );
    socket_state_ = SOCKET_STATE::CREATED;
    return ProbeSocket::RESULT::SUCCESS;
  }
  if( socket_state_ != SOCKET_STATE::CONNECTED )
    return RESULT::INVALID;

  // Run sequencing. Do not break into interactive mode
  if( --runEventCounter_ > 0 )
    return ProbeSocket::RESULT::SUCCESS;

  // Message Loop
  bool continueSim = false;
  do {
    memset( buffer_, 0, SOCKET_BUFFER_SIZE );
    int rc = recv( clientSock_, buffer_, sizeof( buffer_ ), 0 );
    if( rc < 0 )
      return RESULT::RECV_ERROR;
    // std::cout << "<server received>" << buffer_ << std::endl;
    std::stringstream        response;
    std::string              user_input( buffer_ );
    std::vector<std::string> args;
    splitStr( user_input, " ", args );
    CMD cmd = CMD::UNKNOWN;
    if( str2cmd.find( args[0] ) != str2cmd.end() )
      cmd = str2cmd.at( args[0] );
    switch( cmd ) {
    case CMD::CLICONTROL:
      if( args.size() > 1 ) {
        try {
          uint64_t n = std::stol( args[1] );
          probeControl_->cliControl( n );
        } catch( std::exception& e ) {
          std::cout << "Could not set cliControl. " << e.what() << std::endl;
        }
      }
      response << probeControl_->cliControl();
      break;
    case CMD::COMPONENT: response << comp_->getName(); break;
    case CMD::CYCLE: response << comp_->getCurrentSimCycle(); break;
    case CMD::DISCONNECT:
      socket_state_ = SOCKET_STATE::DISCONNECTING;
      response << "disconnecting";
      continueSim = true;
      break;
    case CMD::DUMP:
      // TODO fix socket protocol for any sized buffer transfer
      probeControl_->buf()->render_buffer( response );
      break;
    case CMD::ECHO: {
      std::string s;
      joinStr( 1, args, ".", s );
      response << s;
    } break;

    case CMD::HOSTNAME: response << hostname_; break;
    case CMD::NUMRECS: response << probeControl_->buf()->getNumRecs(); break;
    case CMD::PROBESTATE: response << probeControl_->getProbeStateStr(); break;
    case CMD::RUN:
      if( args.size() > 1 ) {
        try {
          uint64_t n       = std::stol( args[1] );
          continueSim      = true;
          runEventCounter_ = n;
          response << "continuing sim for " << n << " events";
          ;
        } catch( std::exception& e ) {
          std::cout << "could not set number of events. " << e.what() << std::endl;
          response << "?";
        }
      } else {
        runEventCounter_ = 0;
        continueSim      = true;
        response << "continuing sim";
      }
      break;
    case CMD::SPIN:
      kgdbg::spin();
      response << "freed from spin";
      break;
    case CMD::SYNCSTATE: response << probeControl_->getSyncStateStr(); break;
    case CMD::TRIGSTATE: response << probeControl_->buf()->getTrigStateChar(); break;
    case CMD::HELP: {
      std::stringstream r;
      if( args.size() == 1 ) {
        r << "Available commands:\n";
        for( auto s : cmd2str )
          r << " " << s.second << "\n";
        r << "Use 'help <command>' for more detailed information\n";
      } else if( str2cmd.find( args[1] ) != str2cmd.end() ) {
        r << cmd2help.at( str2cmd.at( args[1] ) ) << "\n";
      } else {
        r << "Unknown command: " << args[1] << "\n";
      }
      response << r.str();
    } break;
    case CMD::UNKNOWN: response << "?"; break;
    }

    // std::cout << "<server sending>" << response << std::endl;
    memset( buffer_, 0, SOCKET_BUFFER_SIZE );
    memcpy( buffer_, response.str().c_str(), strlen( response.str().c_str() ) );
    rc = send( clientSock_, buffer_, strlen( buffer_ ), 0 );
  } while( !continueSim );

  return ProbeSocket::RESULT::SUCCESS;
}

void ProbeSocket::splitStr( std::string s, const char* delim, std::vector<std::string>& v ) {
  char* ptr     = s.data();
  char* saveptr = nullptr;
  for( v.clear(); auto token = strtok_r( ptr, delim, &saveptr ); ptr = nullptr )
    v.push_back( token );
}

void ProbeSocket::joinStr( int startpos, std::vector<std::string> v, const char* delim, std::string& s ) {
  s.clear();
  int n = v.size();
  for( int i = startpos; i < n; i++ ) {
    s.append( v[i] );
    if( i < n - 1 )
      s.append( delim );
  }
}

bool ProbeSocket::match( std::string in, CMD cmd ) {
  if( str2cmd.find( in ) == str2cmd.end() )
    return false;
  return ( str2cmd.at( in ) == cmd );
}

}  // namespace SSTDEBUG::Probe
