//
// _PanNet_h_
//
// Copyright (C) 2017-2023 Tactical Computing Laboratories, LLC
// All Rights Reserved
// contact@tactcomplabs.com
//
// See LICENSE in the top level directory for licensing details
//

#ifndef _SST_PANNET_H_
#define _SST_PANNET_H_

// -- Standard Headers
#include <vector>
#include <queue>
#include <map>
#include <unistd.h>

// -- SST Headers
#include <sst/core/sst_config.h>
#include <sst/core/component.h>
#include <sst/core/event.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>
#include <sst/core/interfaces/simpleNetwork.h>

// -- Rev Headers
#include "RevOpts.h"
#include "RevMem.h"
#include "RevLoader.h"
#include "RevProc.h"

namespace SST {
  namespace RevCPU {
    /**
     * panNicEvent : inherited class to handle the individual network events for PanNet
     */
    class panNicEvent : public SST::Event {
    public:

      typedef enum{
        PanBase     = 0b00,               ///< PanPacket: base packet type
        PanStream   = 0b01,               ///< PanPacket: streaming packet type
        PanRsvd     = 0b10,               ///< PanPacket: reserved for future expansion
        PanBOTW     = 0b11                ///< PanPacket: bump on the wire packet
      }PanPacket;

      typedef enum{
        SyncGet         = 0b00000000,     ///< PanOpcode: Synchronous get
        SyncPut         = 0b00000100,     ///< PanOpcode: Synchronous put
        AsyncGet        = 0b00001000,     ///< PanOpcode: Asynchronous get
        AsyncPut        = 0b00001100,     ///< PanOpcode: Asynchronous put
        SyncStreamGet   = 0b0001,         ///< PanOpcode: Synchronous streaming get
        SyncStreamPut   = 0b0101,         ///< PanOpcode: Synchronous streaming put
        AsyncStreamGet  = 0b1001,         ///< PanOpcode: Asynchronous streaming get
        AsyncStreamPut  = 0b1101,         ///< PanOpcode: Asynchronous streaming put
        Exec            = 0b00010000,     ///< PanOpcode: Execute
        Status          = 0b00100000,     ///< PanOpcode: Status
        Cancel          = 0b00110000,     ///< PanOpcode: Cancel
        Reserve         = 0b01000000,     ///< PanOpcode: Reserve
        Revoke          = 0b01010000,     ///< PanOpcode: Revoke
        Halt            = 0b01100000,     ///< PanOpcode: Halt
        Resume          = 0b01110000,     ///< PanOpcode: Resume
        ReadReg         = 0b10000000,     ///< PanOpcode: Read register
        WriteReg        = 0b10010000,     ///< PanOpcode: Write register
        SingleStep      = 0b10100000,     ///< PanOpcode: Single step
        SetFuture       = 0b10110000,     ///< PanOpcode: Set future
        RevokeFuture    = 0b11000000,     ///< PanOpcode: Revoke future
        StatusFuture    = 0b11010000,     ///< PanOpcode: Status future
        Success         = 0b11100000,     ///< PanOpcode: Command success
        Failed          = 0b11110000,     ///< PanOpcode: Failed success
        BOTW            = 0b11            ///< PanOpcode: Bump on the wire
      }PanOpcode;

      /// panNicEvent: standard constructor
      panNicEvent(std::string name)
        : Event(),
        SrcName(name),
          Tag(0), Opcode(PanRsvd), VarArgs(0),
          Size(0), Token(0), Offset(0),
          Addr(0){ }

      /// panNicEvent: rerieve the source name
      std::string getSource() { return SrcName; }

      /// panNicEvent: retrieve the packet type
      panNicEvent::PanPacket getType();

      /// panNicEvent: retrieve the packet tag field
      uint8_t getTag() { return Tag; }

      /// panNicEvent: retrieve the packet opcode
      uint8_t getOpcode() { return Opcode; }

      /// panNicEvent: retrieve the packet varargs
      uint8_t getVarArgs() { return VarArgs; }

      /// panNicEvent: retrieve the packet size
      uint32_t getSize() { return Size; }

      /// panNicEvent: retrieve the packet token
      uint32_t getToken() { return Token; }

      /// panNicEvent: retrieve the packet offset
      uint32_t getOffset() { return Offset; }

      /// panNicEvent: retrieve the packet address
      uint64_t getAddr() { return Addr; }

      /// panNicEvent: retrieve the packet data
      void getData(uint64_t *Out);

      /// panNicEvent: retrieve the source ID
      int getSrc() { return Src; }

      /// panNicEvent: set the source ID
      bool setSrc(int S) { Src = S; return true; }

      /// panNicEvent: set the packet data
      bool setData(uint64_t *In, uint32_t Sz);

      /// panNicEvent: set the tag field
      bool setTag(uint8_t T);

      /// panNicEvent: set the number of varargs
      bool setVarArgs(uint8_t VA);

      /// panNicEvent: set the size in the command packet
      bool setSize(uint32_t Sz);

      /// panNicEvent: set the token for the target packet
      bool setToken(uint32_t T);

      /// panNicEvent: set the offset for the target packet
      bool setOffset(uint32_t O);

      /// panNicEvent: set the address for the command packet
      bool setAddr(uint64_t A);

      /// panNicEvent: get the number of data blocks for the target size
      unsigned getNumBlocks(uint32_t Size);

      /// panNicEvent: return the opcode type as a string
      std::string getOpcodeStr();

      // ------------------------------------------------
      // Packet Building Functions
      // ------------------------------------------------

      /// panNicEvent: build a synchronous get packet
      bool buildSyncGet(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size);

      /// panNicEvent: build a synchronous put packet
      bool buildSyncPut(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size, uint64_t *Data);

      /// panNicEvent: build an asynchronous get packet
      bool buildAsyncGet(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size);

      /// panNicEvent: build an asynchronous put packet
      bool buildAsyncPut(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size, uint64_t *Data);

      /// panNicEvent: build a synchronous streaming get packet
      bool buildSyncStreamGet(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size);

      /// panNicEvent: build a synchronous streaming put packet
      bool buildSyncStreamPut(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size, uint64_t *Data);

      /// panNicEvent: build an asynchronous streaming get packet
      bool buildAsyncStreamGet(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size);

      /// panNicEvent: build an asynchronous streaming put packet
      bool buildAsyncStreamPut(uint32_t Token, uint8_t Tag, uint64_t Addr, uint32_t Size, uint64_t *Data);

      /// panNicEvent: build an execution packet
      bool buildExec(uint32_t Token, uint8_t Tag, uint64_t Addr);

      /// panNicEvent: build a status packet
      bool buildStatus(uint32_t Token, uint8_t Tag, uint16_t Entry);

      /// panNicEvent: build a cancel packet
      bool buildCancel(uint32_t Token, uint8_t Tag, uint16_t Entry);

      /// panNicEvent: build a reserve packet
      bool buildReserve(uint32_t Token, uint8_t Tag);

      /// panNicEvent: build a revoke packet
      bool buildRevoke(uint32_t Token, uint8_t Tag);

      /// panNicEvent: build a halt packet
      bool buildHalt(uint32_t Token, uint8_t Tag, uint16_t Hart);

      /// panNicEvent: build a resume packet
      bool buildResume(uint32_t Token, uint8_t Tag);

      /// panNicEvent: build a readreg packet
      bool buildReadReg(uint32_t Token, uint8_t Tag, uint16_t Hart, uint64_t Reg);

      /// panNicEvent: build a writereg packet
      bool buildWriteReg(uint32_t Token, uint8_t Tag, uint16_t Hart, uint64_t Reg, uint64_t *Data);

      /// panNicEvent: build a singlestep packet
      bool buildSingleStep(uint32_t Token, uint8_t Tag, uint16_t Hart);

      /// panNicEvent: build a set future packet
      bool buildSetFuture(uint32_t Token, uint8_t Tag, uint64_t Addr);

      /// panNicEvent: build a revoke future packet
      bool buildRevokeFuture(uint32_t Token, uint8_t Tag, uint64_t Addr);

      /// panNicEvent: build a status future packet
      bool buildStatusFuture(uint32_t Token, uint8_t Tag, uint64_t Addr);

      /// panNicEvent: build a BOTW packet
      bool buildBOTW(uint32_t Token, uint8_t Tag, uint8_t VarArgs, uint64_t *Args, uint32_t Offset);

      /// panNicEvent: build a success packet
      bool buildSuccess(uint32_t Token, uint8_t Tag);

      /// panNicEvent: build a failed packet
      bool buildFailed(uint32_t Token, uint8_t Tag);

      // ------------------------------------------------
      // Virtual Functions
      // ------------------------------------------------

      /// panNicEvent: vritual function to clone an event
      virtual Event* clone(void) override{
        panNicEvent *ev = new panNicEvent(*this);
        return ev;
      }

    private:
        std::string SrcName;            ///< panNicEvent: Name of the sending device
        int Src;                        ///< panNicEvent: Source ID
        uint8_t Tag;                    ///< panNicEvent: Tag value of the command packet
        uint8_t Opcode;                 ///< panNicEvent: Opcode value of the command packet
        uint8_t VarArgs;                ///< panNicEvent: Variadic arguments for BOTW packet
        uint32_t Size;                  ///< panNicEvent: Size value of the command packet
        uint32_t Token;                 ///< panNicEvent: Token value of the command packet
        uint32_t Offset;                ///< panNicEvent: Offset value for BOTW packet
        uint64_t Addr;                  ///< panNicEvent: Addressing encoding field
        std::vector<uint64_t> Data;     ///< panNicEvent: Data field

    public:
      /// panNicEvent: event serializer
      panNicEvent() : Event() {}

      /// panNicEvent: event serializer
      void serialize_order(SST::Core::Serialization::serializer &ser) override{
        Event::serialize_order(ser);
        ser & SrcName;
        ser & Src;
        ser & Tag;
        ser & Opcode;
        ser & VarArgs;
        ser & Size;
        ser & Token;
        ser & Offset;
        ser & Addr;
        ser & Data;
      }

      /// panNicevent: implements the NIC serialization
      ImplementSerializable(SST::RevCPU::panNicEvent);
    }; // end panNicEvent


    /**
     * panNicAPI: Handles the subcomponent NIC API
     */
    class panNicAPI: public SST::SubComponent{
    private:
      bool isHost;      ///< Determines if this is a host device
      bool isReserved;  ///< Determines if the token is reserved
      uint32_t Token;   ///< Holds the reservation token

    public:
      SST_ELI_REGISTER_SUBCOMPONENT_API(SST::RevCPU::panNicAPI)

      /// panNicAPI: default constructor
      panNicAPI(ComponentId_t id, Params& params)
        : SubComponent(id), isHost(false), isReserved(false), Token(0x00) { }

      /// panNicAPI: set whether this device is a host
      void SetHost(bool host){ isHost = host; }

      /// panNicAPI: determine ehther this is a host device
      bool IsHost() { return isHost; }

      /// panNicAPI: determine if the device is reserved
      bool IsReserved() { return isReserved; }

      /// panNicAPI: retrieve the host token
      uint32_t GetToken() { return Token; }

      /// panNicAPI: check the target token against what is stored
      bool CheckToken(uint32_t T){
        if( !isReserved )
          return false;
        if( Token == T )
          return true;
        return false;
      }

      /// panNicAPI: revoke the token
      void RevokeToken(){
        isReserved = false;
        Token = 0x00;
      }

      /// panNicAPI: set the host token
      bool SetToken(uint32_t T){
        if( !isReserved ){
          Token = T;
          isReserved = true;
          return true;
        }else{
          return false;
        }
      }

      /// panNicAPI: default destructor
      virtual ~panNicAPI() { }

      /// panNicAPI: registers the event handler with the core
      virtual void setMsgHandler(Event::HandlerBase* handler) = 0;

      /// panNicAPI: initializes the network
      virtual void init(unsigned int phase) = 0;

      /// panNicAPI: setup the network
      virtual void setup() { }

      /// panNicAPI: send a message on the network
      virtual void send(panNicEvent *ev, int dest) = 0;

      /// panNicAPI: retrieve the number of potential destinations
      virtual int getNumDestinations() = 0;

      /// panNicAPI: returns the NIC's network address
      virtual SST::Interfaces::SimpleNetwork::nid_t getAddress() = 0;

      /// panNicAPI: determine if the remote ID is a host
      virtual bool IsRemoteHost(SST::Interfaces::SimpleNetwork::nid_t NID) = 0;

      /// panNicAPI: retrieve the number of hosts
      virtual unsigned getNumPEs()  = 0;

      /// panNicAPI: retrieve the endpoint ID of the target map index
      virtual int64_t getHostFromIdx(unsigned Idx) = 0;

    }; // end panNicAPI

    /**
     * PanNet: the PAN network transport module subcomponent
     */
    class PanNet : public panNicAPI {
    public:
      // Register with the SST Core
      SST_ELI_REGISTER_SUBCOMPONENT(
        PanNet,
        "revcpu",
        "PanNet",
        SST_ELI_ELEMENT_VERSION(1, 0, 0),
        "PAN Network Transport Module",
        SST::RevCPU::panNicAPI
      )

      // Register the parameters
      SST_ELI_DOCUMENT_PARAMS(
        {"port", "Port to use, if loaded as an anonymous subcomponent", "network"},
        {"verbose", "Verbosity for output (0 = nothing)", "0"},
        {"host_device", "Determines if this is a host device", "0"}
      )

      // Register the ports
      SST_ELI_DOCUMENT_PORTS(
        {"network", "Port to network", {"simpleNetworkExample.nicEvent"} }
      )

      // Register the subcomponent slots
      SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
        {"iface", "SimpleNetwork interface to a network", "SST::Interfaces::SimpleNetwork"}
      )

      /// PanNet: default constructor
      PanNet(ComponentId_t id, Params& params);

      /// PanNet: default destructor
      virtual ~PanNet();

      /// PanNet: Callback to parent on received messages
      virtual void setMsgHandler(Event::HandlerBase* handler);

      /// PanNet: initialization function
      virtual void init(unsigned int phase);

      /// PanNet: setup function
      virtual void setup();

      /// PanNet: send the event to the destination id
      virtual void send(panNicEvent *ev, int dest);

      /// PanNet: retrieve the number of destinations
      virtual int getNumDestinations();

      /// PanNet: get the endpoint's network address
      virtual SST::Interfaces::SimpleNetwork::nid_t getAddress();

      /// PanNet: clock function
      virtual bool clockTick(Cycle_t cycle);

      /// PanNet: callback function for the SimpleNetwork itnerface
      bool msgNotify(int virtualNetwork);

      /// PanNet: determine if I am a host connected device
      bool IsHost() { return isHost; }

      /// PanNet: determine if the remote ID is a host
      virtual bool IsRemoteHost(SST::Interfaces::SimpleNetwork::nid_t NID){ return hostMap[NID]; }

      /// PanNet: retrieve the number of hosts
      virtual unsigned getNumPEs() { return (unsigned)(hostMap.size()); }

      /// PanNet: retrieve the endpoint ID of the target map index
      virtual int64_t getHostFromIdx(unsigned Idx);

    protected:
      SST::Output *output;                    ///< PanNet: SST output object

      SST::Interfaces::SimpleNetwork * iFace; ///< PanNet: SST network interface

      SST::Event::HandlerBase *msgHandler;    ///< PanNet: SST message handler

      bool initBroadcastSent;                 ///< PanNet: has the init bcast been sent?

      int numDest;                            ///< PanNet: number of SST destinations

      std::queue<SST::Interfaces::SimpleNetwork::Request*> sendQ; ///< PanNet: buffered send queue

    private:
      bool isHost;                            ///< PanNet: Determines if this is a host device

      std::map<SST::Interfaces::SimpleNetwork::nid_t, bool> hostMap;///< PanNet: Maps an endpoint ID to whether it is a host device

    }; // end PanNet

  } // namespace RevCPU
} // namespace SST

#endif

// EOF
