/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 * Copyright 2010,2011 Jesse Allen
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Filename    : multiplayer.h
// Description : Multiplayer game support.

#ifndef __MULTIPLAYER_H
#define __MULTIPLAYER_H

#include <MPTYPES.h>
#include <player_desc.h>
#include <ODYNARRB.h>
#include <stdint.h>
#include <network.h>
#include <queue>

#define MP_SERVICE_PROVIDER_NAME_LEN 64
#define MP_SESSION_NAME_LEN 64
#define MP_PASSWORD_LEN 32
#define MP_RECV_BUFFER_SIZE 0x2000
#define MP_GAME_LIST_SIZE 10
#define MP_LADDER_LIST_SIZE 6

#define MP_STATUS_IDLE 0
#define MP_STATUS_CONNECTING 1
#define MP_STATUS_PREGAME 2
#define MP_STATUS_INGAME 3
#define MP_STATUS_REQ_GAME_LIST 4
#define MP_STATUS_REQ_LADDER 5

enum
{
	MPMSG_GAME_BEACON = 0x1f4a0001,
	MPMSG_REQ_GAME_LIST,
	MPMSG_GAME_LIST,
	MPMSG_VERSION_ACK,
	MPMSG_VERSION_NAK,
	MPMSG_CONNECT,
	MPMSG_CONNECT_ACK,
	MPMSG_REQ_LADDER,
	MPMSG_LADDER,
	MPMSG_NEW_PEER_ADDRESS,
};

struct mp_msg
{
	uint32_t player_id;
	uint16_t size;
	struct packet_header header;
	char *data;
};

#pragma pack(1)
struct MsgHeader
{
	uint32_t msg_id;
};

struct MsgGameBeacon
{
	uint32_t msg_id;
	char name[MP_SESSION_NAME_LEN];
	char password;
};

struct MsgRequestGameList
{
	uint32_t msg_id;
	uint32_t ack;
};

struct remote_game
{
	char name[MP_SESSION_NAME_LEN];
	char password;
	uint32_t host;
	uint16_t port;
};

struct MsgGameList
{
	uint32_t msg_id;
	uint32_t page;
	uint32_t total_pages;
	struct remote_game list[MP_GAME_LIST_SIZE];
};

struct MsgVersionAck
{
	uint32_t msg_id;
};

struct MsgVersionNak
{
	uint32_t msg_id;
	uint32_t major;
	uint32_t medium;
	uint32_t minor;
};

struct MsgConnect
{
	uint32_t msg_id;
	uint32_t player_id;
	char name[MP_SESSION_NAME_LEN];
	char password[MP_SESSION_NAME_LEN];
};

struct MsgConnectAck
{
	uint32_t msg_id;
	uint32_t your_id;
};

struct MsgRequestLadder
{
	uint32_t msg_id;
};

struct ladder_entry
{
	char name[MP_PLAYER_NAME_LEN];
	uint16_t wins;
	uint16_t losses;
	int32_t score;
};

struct MsgLadder
{
	uint32_t msg_id;
	struct ladder_entry list[MP_LADDER_LIST_SIZE];
};

struct MsgNewPeerAddress
{
	uint32_t msg_id;
	uint32_t player_id;
	uint32_t host;
	uint16_t port;
};
#pragma pack()

enum ProtocolType
{
	None = 0,
	IPX = 1,
	TCPIP = 2,
	Modem = 4,
	Serial = 8
};

struct SessionDesc
{
	char session_name[MP_SESSION_NAME_LEN+1];
	char password[MP_SESSION_NAME_LEN+1];
	uint32_t id;
	struct inet_address address;

	SessionDesc();
	SessionDesc(const SessionDesc &);
	SessionDesc& operator= (const SessionDesc &);

	char *name_str() { return session_name; };
	uint32_t session_id() { return id; }
};


class MultiPlayer
{
private:

	int               init_flag;
	int               lobbied_flag;
	int               status;

	ProtocolType      supported_protocols;
	DynArrayB         current_sessions;
	SessionDesc       joined_session;
	Network           *network;

	uint32_t          my_player_id;
	PlayerDesc        *my_player;

	int               host_flag;
	int               allowing_connections;
	int               max_players;

	PlayerDesc        *player_pool[MAX_NATION];

	char *            recv_buf;
	std::queue<struct mp_msg *> recv_queue;

	int               game_sock;
	int               standard_port;

	struct inet_address lan_broadcast_address;
	struct inet_address remote_session_provider_address;

	int use_remote_session_provider;
	int update_available;

public:

	MultiPlayer();
	~MultiPlayer();

	void   init(ProtocolType);
	void   deinit();
	bool   is_initialized() const { return init_flag != 0; }

	// ------- functions on lobby -------- //
	int    init_lobbied(int maxPlayers, char * cmdLine);
	int    is_lobbied(); // return 0=not lobbied, 1=auto create, 2=auto join, 4=selectable
	char * get_lobbied_name(); // return 0 if not available

	// ------- functions on network protocols ------ //
	void   poll_supported_protocols(); // can be called before init
	bool   is_protocol_supported(ProtocolType);
	int    is_update_available();
	int    is_pregame();
	void   game_starting();

	// ------- functions on session --------//
	int    set_remote_session_provider(const char *server);
	int    poll_sessions();
	void   sort_sessions(int sortType);
	int    create_session(char *sessionName, char *password, char *playerName, int maxPlayers);
	int    join_session(int i, char *password, char *playerName);
	void   close_session();
	SessionDesc* get_session(int i);

	// -------- functions on player management -------//
	int         add_player(char *name, uint32_t id);
	void        set_my_player_id(uint32_t id);
	void        set_player_name(uint32_t id, char *name);
	void        delete_player(uint32_t id);
	void        poll_players();
	PlayerDesc* get_player(int i);
	PlayerDesc* search_player(uint32_t playerId);
	int         is_player_connecting(uint32_t playerId);
	int         get_player_count();
	uint32_t    get_my_player_id() const { return my_player_id; }
	void        set_peer_address(uint32_t who, struct inet_address *address);

	// ------- functions on message passing ------//
	int    send(uint32_t to, void * data, uint32_t msg_size);
	int    send_stream(uint32_t to, void * data, uint32_t msg_size);
	char * receive(uint32_t * from, uint32_t * to, uint32_t * size, int *sysMsgCount=0);
	char * receive_stream(uint32_t * from, uint32_t * to, uint32_t * size, int *sysMsgCount=0);

	int show_leader_board();
	void yield();

private:
	int open_port();
	int open_standard_port(int fallback);

	int send_nonseq_msg(int sock, char *msg, int msg_size, struct inet_address *to);
	int send_system_msg(int sock, char *msg, int msg_size, struct inet_address *to);

	int create_player(char *name, struct inet_address *address);
	int get_player_id(struct inet_address *address);
	int check_duplicates(struct inet_address *address);
	void msg_game_beacon(MsgGameBeacon *m, struct inet_address *addr);
	int msg_game_list(MsgGameList *m, int last_ack, struct inet_address *addr);
	void msg_version_nak(MsgVersionNak *p, struct inet_address *addr);

	void udp_accept_connections(struct packet_header *h, struct inet_address *address);
	void send_game_beacon();

	void yield_recv();
	void yield_connecting();
	void yield_pregame();
};

extern MultiPlayer mp_obj;

#endif // __MULTIPLAYER_H

