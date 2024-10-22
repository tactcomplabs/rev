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

#ifndef SST_DEBUG_PROBE_H
#define SST_DEBUG_PROBE_H

// -- Standard Headers
#include <assert.h>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

// -- SST Headers
#include <sst/core/component.h>
#include <sst/core/output.h>
#include <sst/core/sst_config.h>

namespace SSTDEBUG::Probe {

class ProbeBufCtl;
class ProbeSocket;

enum class SyncState { INVALID, IDLE, WAIT, ACTIVE };

static const std::map<SyncState, std::string> syncState2Str{
  {SyncState::INVALID, "invalid"},
  {   SyncState::IDLE,    "idle"},
  {   SyncState::WAIT,    "wait"},
  { SyncState::ACTIVE,  "active"}
};

/// ProbeState only valid when SyncState is ACTIVE
enum class ProbeState { IDLE, RESTART, PRE_SAMPLING, POST_SAMPLING, WAIT };

static const std::map<ProbeState, std::string> probeState2Str{
  {         ProbeState::IDLE,          "idle"},
  {      ProbeState::RESTART,       "restart"},
  { ProbeState::PRE_SAMPLING,  "pre-sampling"},
  {ProbeState::POST_SAMPLING, "post-sampling"},
  {         ProbeState::WAIT,          "wait"}
};

union Actions {
  uint64_t v = 0;

  struct {
    // TODO render_mode: binary, text, json, custom
    uint64_t flush2stdout : 1;  // flush to console
    uint64_t flush2file   : 1;  // flush buffer to file
    uint64_t repeat       : 1;  // reset probe trigger and restart
    uint64_t cont         : 1;  // continue post-trig sampling. reset delay counter if used.
    uint64_t shutdown     : 1;  // shutdown simulation
  } f;
};

// Breaking into CLI mode options:
// 0b0100_0000:  Every checkpoint
// 0b0010_0000:  Every checkpoint when probe is active
// 0b0001_0000:  Every checkpoint probe state change
// 0b0000_0100:  Every probe sample
// 0b0000_0010:  Every probe sample from trigger onward
// 0b0000_0001:  Every probe state change
union CLI_CTRL {
  uint64_t v = 0;

  struct {
    uint64_t probe_states       : 1;  // [0] Every probe state change
    uint64_t probe_trig2samples : 1;  // [1] Every probe sample from trigger forward
    uint64_t probe_samples      : 1;  // [2] Every probe sample
    uint64_t u0                 : 1;  // [3] unused
    uint64_t chkpt_states       : 1;  // [4] Every checkpoint when sync state changes
    uint64_t chkpt_active       : 1;  // [5] Every checkpoint when probe active
    uint64_t chkpt_all          : 1;  // [6] Every checkpoint
    uint64_t u1                 : 1;  // [7] Unused
    uint64_t chkpt_cli          : 1;  // [8] [1] entered CLI from checkpoint, [0] entered CLI from component probe
  } f;

  CLI_CTRL( uint64_t c ) { v = c & 0x077; };
};

class ProbeControl {

public:
  // TODO component should inherit from ProbeControl
  ProbeControl(
    SST::Component* comp,
    SST::Output*    out,
    int             probeMode,
    int             probeStartCycle,
    int             probeEndCycle,
    int             probeBufferSize,
    int             probePort,
    int             probePostDelay,
    uint64_t        cliControl
  );
  virtual ~ProbeControl();
  /// Disallow copying and assignment
  ProbeControl( const ProbeControl& )            = delete;
  ProbeControl& operator=( const ProbeControl& ) = delete;
  /// child class provides controls to buffer
  void setBufferControls( std::shared_ptr<ProbeBufCtl> );
  /// Call back for sync points for high level controller updates
  void updateSyncState( int cycle );
  /// Called at end of component's clock cycle and end of updateSyncState
  void updateProbeState( int cycle );
  /// handle any requested sync point actions
  void handleSyncPointActions();
  /// Detect trigger to transition between pre and post sampling phase
  void trigger( bool cond );  // TODO pick a more specific name that differentiates this from other triggers
  /// Indicate data has been sampled.
  void sample();

  /// Avoid context switch by checking active state before probing
  inline bool active() const { return syncState_ == SyncState::ACTIVE; }

  /// Avoid context switch for trigger check
  inline bool triggering() const { return probeState_ == ProbeState::PRE_SAMPLING; }

  ///Avoid context switch on sampling
  inline bool sampling() const {
    return ( probeState_ == ProbeState::PRE_SAMPLING ) || ( probeState_ == ProbeState::POST_SAMPLING );
  }

  inline std::string const getSyncStateStr() { return syncState2Str.at( syncState_ ); }

  inline std::string const getProbeStateStr() { return probeState2Str.at( probeState_ ); }

  inline uint64_t cliControl() { return cliControl_.v; }

  inline void cliControl( uint64_t v ) { cliControl_.v = v; }

public:
  /// Access functions for testing and CLI support
  SST::Component* comp() { return comp_; };

  std::shared_ptr<ProbeBufCtl> buf() { return probeBufCtl_; }

  /// CLI server may run only when active and a probe port has been provided.
  void updateCLI();

private:
  SST::Component*              comp_;                                 ///< Component associated with this controller
  SST::Output*                 out_;                                  ///< Component output stream
  SyncState                    syncState_      = SyncState::INVALID;  ///< State managed at sync points
  SyncState                    lastSyncState_  = SyncState::INVALID;  /// < Saved sync state
  int                          syncCycle       = 0;                   ///< Cycle passed to updateSyncState
  ProbeState                   probeState_     = ProbeState::IDLE;    ///< State managed by component level probe
  ProbeState                   lastProbeState_ = ProbeState::IDLE;    /// <saved probe state
  std::unique_ptr<ProbeSocket> probeSocket_;                          ///< CLI Probe Socket Server
  Actions                      syncActions_  = {};                    ///< Common actions to perform at checkpoint
  Actions                      probeActions_ = {};                    ///< Common actions to perform on probe event
  std::shared_ptr<ProbeBufCtl> probeBufCtl_;                          ///< Controls for probe buffer

  // -- Component probe parameters
  int      mode_;                ///< 0-disable, 1-checkpoint-mode, >1-reserved
  int      startCycle_;          ///< When checkpoint >= probeStartCycle sampling begins
  int      endCycle_;            ///< Cycle to disable sampling. When 0, no limit
  int      bufferSize_;          ///< initial number of entries for circular buffers.
  int      port_;                ///< socket assignment for debug probe port  ( 0 = None )
  int      postDelayCounter_;    ///< Post trigger delay (-1 to post-trigger sample until checkpoint)
  int      postDelayInitCount_;  ///< Delay counter initial value
  CLI_CTRL cliControl_ = 0;      ///< controls for breaking into interactive mode
  bool     useDelayCounter_;     ///< when 0 post-trigger sampling continues until checkpoint.
};

// splits generic control from templatized data capture for Probe Buffer
class ProbeBufCtl {
public:
  enum TRIGGER_STATE : int {
    CLEAR     = 0,  // not triggered
    TRIGREC   = 1,  // current sample associated with trigger
    TRIGGERED = 2,  // trigger has occurred
    OVERRUN   = 3,  // trigger has occurred but buffer record overwritten
  };

  const std::map<TRIGGER_STATE, char> trig2char{
    {    CLEAR, '-'},
    {  TRIGREC, '!'},
    {TRIGGERED, '+'},
    {  OVERRUN, 'o'}
  };
  ProbeBufCtl( int sz );
  void         reset_buffer();                                     // effectively clear buffer (e.g. after flush)
  void         reset_trigger();                                    // Clear trigger states
  void         markAsTriggerRec();                                 // Set TRIGREC state to enable special capture
  void         render_buffer( std::ostream& os );                  // iterate over buffer for output
  virtual void render( std::ostream& os, int idx, char pfx ) = 0;  // print a rec to ostream
  virtual void render_trigger_rec( std::ostream&, char pfx ) = 0;  // print the saved trigger rec

  // cli support
  char getTrigStateChar() { return trig2char.at( state ); };

  int getNumRecs() { return num_recs; }

protected:
  void                       capture();         // Called by child after capture record
  int                        sz_;               // defined size
  int                        num_recs     = 0;  // number of valid entries (max is sz)
  int                        cur          = 0;  // index to buffer entry to be written
  int                        first        = 0;  // index of oldest data written
  int                        samples_lost = 0;  // number of samples sampled but overwritten in circular buffer
  TRIGGER_STATE              state;             // current state of triggering sequence
  std::vector<TRIGGER_STATE> tags;              // state associated with each entry.
};

// Simple template wrapper for buffer data
template<typename T>
class ProbeBuffer : public ProbeBufCtl {
public:
  ProbeBuffer( int sz ) : ProbeBufCtl( sz ) { buf.resize( sz ); };

  virtual ~ProbeBuffer() {};

  void capture( T& rec ) {
    ProbeBufCtl::capture();  // update pointers and trigger capture detection
    assert( cur < sz_ );
    buf.at( cur ) = rec;
    if( tags[cur] == TRIGREC )
      trigger_rec = rec;
  }

  void render( std::ostream& os, int idx, char pfx ) override {
    assert( idx < sz_ );
    os << pfx << ' ' << buf.at( idx );
  }

  void render_trigger_rec( std::ostream& os, char pfx ) override { os << pfx << ' ' << trigger_rec; }

private:
  std::vector<T> buf;          // the circular buffer
  T              trigger_rec;  // copy of record associated with triggered cycle
};

class ProbeSocket {

public:
  static const int SOCKET_BUFFER_SIZE = 4096;
  enum class RESULT : int {
    SUCCESS         = 0,
    INVALID         = -1,
    CREATION_ERROR  = -2,
    BINDING_ERROR   = -3,
    ACCEPTING_ERROR = -4,
    RECV_ERROR      = -5,
    SEND_ERROR      = -6,
  };
  enum class SOCKET_STATE : int {
    INVALID       = 0,
    CREATED       = 1,
    DISCONNECTING = 2,
    CONNECTED     = 3,
  };
  enum class CMD : int {
    CLICONTROL,
    COMPONENT,
    CYCLE,
    DISCONNECT,
    DUMP,
    ECHO,
    HELP,
    HOSTNAME,
    NUMRECS,
    PROBESTATE,
    RUN,
    SPIN,
    SYNCSTATE,
    TRIGSTATE,
    UNKNOWN,
  };
  const std::map<CMD, const std::string> cmd2str{
    {CMD::CLICONTROL, "clicontrol"},
    { CMD::COMPONENT,  "component"},
    {     CMD::CYCLE,      "cycle"},
    {CMD::DISCONNECT, "disconnect"},
    {      CMD::DUMP,       "dump"},
    {      CMD::ECHO,       "echo"},
    {      CMD::HELP,       "help"},
    {  CMD::HOSTNAME,   "hostname"},
    {   CMD::NUMRECS,    "numrecs"},
    {CMD::PROBESTATE, "probestate"},
    {       CMD::RUN,        "run"},
    {      CMD::SPIN,       "spin"},
    { CMD::SYNCSTATE,  "syncstate"},
    { CMD::TRIGSTATE,  "trigstate"},
    {   CMD::UNKNOWN,           ""},
  };
  const std::map<const std::string, CMD> str2cmd{
    {"clicontrol", CMD::CLICONTROL},
    { "component",  CMD::COMPONENT},
    {     "cycle",      CMD::CYCLE},
    {"disconnect", CMD::DISCONNECT},
    {      "dump",       CMD::DUMP},
    {      "echo",       CMD::ECHO},
    {      "help",       CMD::HELP},
    {  "hostname",   CMD::HOSTNAME},
    {   "numrecs",    CMD::NUMRECS},
    {"probestate", CMD::PROBESTATE},
    {       "run",        CMD::RUN},
    {      "spin",       CMD::SPIN},
    { "syncstate",  CMD::SYNCSTATE},
    { "trigstate",  CMD::TRIGSTATE},
    {         "?",       CMD::HELP},
    {          "",    CMD::UNKNOWN}
  };
  const std::map<CMD, const std::string> cmd2help{
    {CMD::CLICONTROL,
     "Controls for breaking into interactive mode\n"
     "clicontrol   : returns current value\n"
     "clicontrol n : sets clicontrol to 'n' (base 10 only)\n"
     " 0b0100_0000 : 0x40 : 64  Every checkpoint\n"
     " 0b0010_0000 : 0x20 : 32  Every checkpoint when probe is active\n"
     " 0b0001_0000 : 0x10 : 16  Every checkpoint sync state change\n"
     " 0b0000_0100 : 0x04 : 04  Every probe sample\n"
     " 0b0000_0010 : 0x02 : 02  Every probe sample from trigger onward\n"
     " 0b0000_0001 : 0x01 : 01  Every probe state change\n"
     "bit 8: (read-only) CLI_MODE\n"
     "[1] CLI invoked from checkpoint probe\n"
     "[0] CLI invoked from component probe\n"                                 },
    { CMD::COMPONENT,                 "get name of the component being probed"},
    {     CMD::CYCLE,     "get current simulation cycle seen by the component"},
    {CMD::DISCONNECT, "disconnect from probe and allow simulation to complete"},
    {      CMD::DUMP,                           "dump the probe sample buffer"},
    {      CMD::ECHO,              "(test) return args following echo command"},
    {      CMD::HELP,                    "provide helpful information to user"},
    {  CMD::HOSTNAME,          "get name of the host running the probe server"},
    {   CMD::NUMRECS,                            "number of records in buffer"},
    {CMD::PROBESTATE,
     "query current component probe state\n"
     "idle, pre-sampling, post-sampling, wait\n"                              },
    {       CMD::RUN,
     "run   : continue simulation until the next event defined by clicontrols\n"
     "run N : run through N events\n"                                         },
    {      CMD::SPIN,              "(test) enter spin loop for gdb connection"},
    { CMD::SYNCSTATE,
     "query current simulator sync state\n"
     "invalid, wait, active, idle\n"                                          },
    { CMD::TRIGSTATE,
     "query current buffer trigger state\n"
     "- pre-trigger\n! trigger record\n+ post-trigger\no buffer overrun"      },
    {   CMD::UNKNOWN,                                        "Unknown command"},
  };

  ProbeSocket( int port, ProbeControl* probeControl, SST::Component* comp, SST::Output* out );
  virtual ~ProbeSocket();
  /// Create a valid socket
  RESULT create();
  /// Wait for connection from client
  RESULT connect();
  /// Handle client commands and return a response
  virtual RESULT cli_handler();

  /// Utility for client to check state before entering CLI loop
  inline bool connected() { return socket_state_ == SOCKET_STATE::CONNECTED; }

private:
  int             port_ = 0;
  ProbeControl*   probeControl_;
  SST::Component* comp_;
  SST::Output*    out_;
  SOCKET_STATE    socket_state_ = SOCKET_STATE::INVALID;
  char            buffer_[SOCKET_BUFFER_SIZE];
  int             serverSock_      = 0;
  int             clientSock_      = 0;
  std::string     hostname_        = "invalid";
  int             runEventCounter_ = 0;  // counter for successive RUN commands
  //TODO does SST have utility class for these things?
  // courtesy of RevOpts.h
  void splitStr( std::string s, const char* delim, std::vector<std::string>& v );
  void joinStr( int startpos, std::vector<std::string> v, const char* delim, std::string& s );
  bool match( std::string in, CMD cmd );

};  //class ProbeSocket

}  // namespace SSTDEBUG::Probe
#endif /* SST_DEBUG_PROBE_H */
