#!/usr/bin/python3
#
# Copyright (C) 2017-2024 Tactical Computing Laboratories, LLC
# All Rights Reserved
# contact@tactcomplabs.com
#
# See LICENSE in the top level directory for licensing details
#
# run-client.py

import argparse
import socket

from enum import Enum

BUFFER_SIZE = 4096
HOST = "127.0.0.1"

class State(Enum):
    INIT = 0
    RUN = 1
    DISCONNECT = 2

def decode(msg):
    global state
    cmd = msg.lower().strip()
    if cmd=="disconnect" or cmd=="quit" or cmd=="exit" or cmd=="bye":
        state = State.DISCONNECT

def client_program(probePort):
    global state
    client_socket = socket.socket()
    print(f"Connecting to {HOST}:{probePort}")
    client_socket.connect( (HOST, probePort) )

    # socket server identification
    state = State.INIT
    divider = "##################################"
    print(divider)

    cmd = "hostname"
    client_socket.send(cmd.encode())
    servername = client_socket.recv(BUFFER_SIZE).decode()
    print(f"# {cmd} = {servername}")

    cmd = "component"
    client_socket.send(cmd.encode())
    comp = client_socket.recv(BUFFER_SIZE).decode()
    print(f"# {cmd} = {comp}")

    cmd = "cycle"
    client_socket.send(cmd.encode())
    cycle = client_socket.recv(BUFFER_SIZE).decode()
    print(f"# {cmd} = {cycle}")

    cmd = "clicontrol"
    client_socket.send(cmd.encode())
    clicontrol = int(client_socket.recv(BUFFER_SIZE).decode(),10)
    print(f"# {cmd} = 0x{clicontrol:x}")

    print(divider)

    state = State.RUN
    while state == State.RUN:
        client_socket.send("clicontrol".encode())
        cctl = int(client_socket.recv(BUFFER_SIZE).decode(),10)
        id = "comp"
        if (cctl & 0x100):
            id = "chkpt"
            client_socket.send("syncstate".encode())
        else:
            client_socket.send("probestate".encode())
        stat = client_socket.recv(BUFFER_SIZE).decode()
        client_socket.send("cycle".encode())
        cycle = client_socket.recv(BUFFER_SIZE).decode()
        PROMPT = f"[{id}:{stat}:{comp}:{cycle}]> "
        msg_len=0;
        while msg_len<1:
            msg = input(PROMPT).lstrip(' ').rstrip(' ')
            msg_len = len(msg)
        decode(msg)
        if state == State.DISCONNECT:
            msg = "disconnect"
        client_socket.send(msg.encode())
        data = client_socket.recv(BUFFER_SIZE).decode()
        print(data)

    client_socket.close()

if __name__ == '__main__':

    parser = argparse.ArgumentParser(description="dbgcli-client")
    parser.add_argument("--probePort", type=int, help="sst probe starting socket. 0=None", required=True)
    args = parser.parse_args()
    print("dbgcli-client configuration:")
    for arg in vars(args):
        print("\t", arg, " = ", getattr(args, arg))

    try:
        client_program(args.probePort)
    except Exception as err:
        print(f"disconnected: {err=}, {type(err)=}")
    else:
        print("dbgcli-client completed normally")






