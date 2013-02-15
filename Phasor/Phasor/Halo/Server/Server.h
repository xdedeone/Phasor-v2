//! \file Server.h
#pragma once

#include "../../../Common/Types.h"
#include "MapLoader.h"

// declared in ../../Commands.h
enum e_command_result;

namespace halo { 
	struct s_player;
	struct s_player_structure;
namespace server
{
	//---------------------------------------------------------------------
	// DEFINITIONS
	// 
	#pragma pack(push, 1)
	//! Represents Halo's connection information structure (ip, port, keys etc)
	struct s_connection_info
	{
		BYTE ip[4];
		WORD port;
		WORD pad;
		BYTE unk[0x148]; // handshake stuff etc, don't care atm.
	};
	static_assert(sizeof(s_connection_info) == 0x150, "incorrect s_connection_info");

	//! Represents an entry in Halo's machine table
	struct s_machine_info
	{
		s_connection_info*** con_info_ptr; 
		DWORD zero;
		DWORD zero1;
		WORD playerNum; // used for rcon etc
		WORD seven;
		BYTE unk[0x42];
		char key[10]; // only 7 chars long tho.. i think rest is padding
		DWORD id_hash;

		s_connection_info* get_con_info()
		{
			return **con_info_ptr;
		}
#ifdef PHASOR_CE
		char unk1[0x20]
		char ip[0x20]
		char cd_key_hash[0x20];
		char unk2[0x2c]
#endif
	};
	static_assert(sizeof(s_machine_info) == MACHINE_ENTRY_SIZE, "incorrect s_machine_info");

	//! Represents an entry in Halo's connection player table
	struct s_presence_item
	{
		wchar_t name[12];
		DWORD idk;
		BYTE machineId;
		BYTE status;
		BYTE team;
		BYTE playerId;
	};
	static_assert(sizeof(s_presence_item) == 0x20, "incorrect s_presence_item");

	//! Server related items (name, gametype, players, machines)
	struct s_server_info
	{
		void* unk_ptr;
		WORD state; // 0 inactive, 1 game, 2 results
		WORD unk1;
		wchar_t server_name[0x42];
		char map_name[0x80];
		wchar_t gametype[0x18];
		BYTE unk2[0x69];
		BYTE max_players;
		WORD unk3;
		BYTE cur_players;
		BYTE unk4;
		s_presence_item player_data[16];
		BYTE unk5[14];
		s_machine_info machine_table[16];
	};
	#pragma pack(pop)

	// Stream for sending server messages
	class SayStream : public COutStream
	{
	protected:
		virtual bool Write(const std::wstring& str);

	public:
		SayStream() {}

		virtual std::unique_ptr<COutStream> clone() const override
		{
			return std::unique_ptr<COutStream>(new SayStream());
		}
	};

	//! Stream used for server messages.
	extern SayStream say_stream;

	/*! \brief Starts a new game on the specified map.
	 * \param map The map to player. */
	void StartGame(const char* map);

	/*! \brief Send a chat message to the player
	 *	\param player The player to send the message to
	 *	\param str The message to send.	 */
	void MessagePlayer(const s_player& player, const std::wstring& str);
	
	/*! \brief Send a console message to the player
	 *	\param player The player to send the message to
	 *	\param str The message to send.	 */
	bool ConsoleMessagePlayer(const s_player& player, const std::wstring& str);
	
	/*! \brief Notifies the server that a player has changed team (syncs it)
	 *	\param player The player who changed team. */
	void NotifyServerOfTeamChange(const halo::s_player& player);

	void ExecuteServerCommand(const std::string& command);

	// Gets the player's ip
	bool GetPlayerIP(const s_player& player, std::string* ip, WORD* port);
	
	// Gets the player's hash
	bool GetPlayerHash(const s_player& player, std::string& hash);
	
	// Get the player's machine info (ip struct etc)
	s_machine_info* GetMachineData(const s_player& player);
	
	/*! \brief Get the player executing the current command
	 * \return The player executing the command, or NULL if no player. */
	halo::s_player* GetPlayerExecutingCommand();

	void SetExecutingPlayer(halo::s_player* player);

	/*! \brief Get the server struct
		\return The server struct.*/
	s_server_info* GetServerStruct();


	// --------------------------------------------------------------------
	// Events
	
	// Called for console events (exit etc)
	/*! \brief Called for Windows related console events (ie closing the server)
	 *	\param fdwCtrlType The type of event. */
	void __stdcall ConsoleHandler(DWORD fdwCtrlType);

	/*! \brief Called every cycle to read input from the user. I use it for timers. */
	void __stdcall OnConsoleProcessing();

	/*! \brief Called when a client sends its update packet (new pos, fire etc)
	 *	\param m_player The memory address of the player who is updating.*/
	void __stdcall OnClientUpdate(s_player_structure* m_player);

	/*! \brief Called to process a server command, after the password has been validated.
	 *	\param command The command being executed.
	 *	\return Value indicating whether or not Halo should process the event.*/
	e_command_result __stdcall ProcessCommand(char* command);

	/*! \brief Called when a new game starts.
	 *	\param map The map the game is running.*/
	void __stdcall OnNewGame(const char* map);

	/*! \brief Called when a game stage ends.
	 *	\param mode Specifies which stage is ending (game, post-game, scorecard) */
	void __stdcall OnGameEnd(DWORD mode);

	/*! \brief Called when a map is being loaded.
	 *	\param loading_map The map being loaded, which can be changed.
	 *	\return Boolean indicating whether or not the map was changed.*/
	bool __stdcall OnMapLoad(maploader::s_mapcycle_entry* loading_map);

	/*! \brief Called when halo wants to print a message to the console.
	 * \todo make sure I send console message if there is an executing player.*/
	void __stdcall OnHaloPrint(char* msg);

	/*! \brief Called when halo checks if the specified hash is banned.
	 *	\param hash The hash being checked.
	 *	\return Boolean indicating whether or not the player is allowed to join.*/
	bool __stdcall OnHaloBanCheck(char* hash);

	// Called when the server info is about to be broadcast
	bool __stdcall OnVersionBroadcast(DWORD arg1, DWORD arg2);

} }