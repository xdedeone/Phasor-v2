#include "Game.h"
#include "../HaloStreams.h"
#include "../Server/MapLoader.h"
#include "../Player.h"
#include "../AFKDetection.h"
#include "../Alias.h"
#include "../../Globals.h"
#include "../../../Common/MyString.h"
#include "../../../ScriptingEvents.h"
#include "../tags.h"
#include <vector>

namespace halo { namespace game {
	typedef std::unique_ptr<s_player> s_player_ptr;
	s_player_ptr PlayerList[16];

	inline bool valid_index(DWORD playerIndex)
	{
		return playerIndex >= 0 && playerIndex < 16;
	}

	// Find the player based on their memory id.
	s_player* GetPlayer(int index)
	{
		if (!valid_index(index)) return NULL;
		return PlayerList[index] ? PlayerList[index].get() : NULL;
	}

	// Find the player based on their player/rcon/machine id.
	s_player* GetPlayerFromRconId(unsigned int playerNum)
	{
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i] && PlayerList[i]->mem->playerNum == playerNum)
				return PlayerList[i].get();
		}
		return NULL;
	}

	s_player* GetPlayerFromAddress(s_player_structure* player)
	{
		for (int i = 0; i < 16; i++) {
			if (PlayerList[i] && PlayerList[i]->mem == player)
				return PlayerList[i].get();
		}
		return NULL;
	}

	// Called when a game stage ends
	void OnGameEnd(DWORD mode)
	{
		switch (mode)
		{
		case 1: // just ended (in game scorecard is shown)
			{
				afk_detection::Disable();
				g_GameLog->WriteLog(kGameEnd, L"The game has ended.");
				*g_PrintStream << "The game is ending..." << endl;

			} break;
		}	

		scripting::events::OnGameEnd(mode);
	}

	// Called when a new game starts
	void OnNewGame(const char* map)
	{
		afk_detection::Enable();
		halo::BuildTagCache();

		for (int i = 0; i < 16; i++) PlayerList[i].reset();
		g_GameLog->WriteLog(kGameStart, "A new game has started on map %s", map);

		scripting::events::OnNewGame(map);
	}

	// Called when a player joins (after verification).
	void __stdcall OnPlayerWelcome(DWORD playerId)
	{
		if (!valid_index(playerId)) {
			*g_PhasorLog << "Player joined with invalid index??" << endl;
			return;
		}
		PlayerList[playerId].reset(new s_player(playerId));
		s_player* player = GetPlayer(playerId);	
		g_GameLog->WriteLog(kPlayerJoin, L"%s (%s ip: %s:%i)", 
			player->mem->playerName, WidenString(player->hash).c_str(),
			WidenString(player->ip).c_str(), player->port);
		alias::OnPlayerJoin(*player);
		scripting::events::OnPlayerJoin(*player);
	}

	// Called when a player quits
	void __stdcall OnPlayerQuit(DWORD playerId)
	{
		if (!valid_index(playerId)) {
			*g_PhasorLog << "Player left with invalid index??" << endl;
			return;
		}
		s_player* player = GetPlayer(playerId);

		if (player) {			
			g_GameLog->WriteLog(kPlayerLeave, L"%s (%s)", 
				player->mem->playerName, WidenString(player->hash).c_str()
				);

			scripting::events::OnPlayerLeave(*player);
			PlayerList[playerId].reset();
		}
	}

	DWORD __stdcall OnTeamSelection(DWORD cur_team, server::s_machine_info* machine)
	{
		DWORD new_team = cur_team;
		scripting::events::OnTeamDecision(cur_team, new_team);
		return new_team;
	}

	/*! \todo add checks when sv_teams_change or w/e is enabled */
	bool __stdcall OnTeamChange(DWORD playerId, DWORD new_team)
	{
		s_player* player = GetPlayer(playerId);
		bool allow = true;

		if (player && new_team != player->mem->team) {
			allow = scripting::events::OnTeamChange(*player, true, new_team,
				player->mem->team);

			g_GameLog->WriteLog(kPlayerChange, L"Blocked %s from changing team.", 
				player->mem->playerName);
		}
		return allow;
	}

	// Called when a player is about to spawn (object already created)
	void __stdcall OnPlayerSpawn(DWORD playerId, ident m_objectId)
	{
		halo::s_player* player = GetPlayer(playerId);
		if (!player) return;
		scripting::events::OnPlayerSpawn(*player, m_objectId);
	}

	// Called after the server has been notified of a player spawn
	void __stdcall OnPlayerSpawnEnd(DWORD playerId, ident m_objectId)
	{
		halo::s_player* player = GetPlayer(playerId);
		if (!player) return;
		scripting::events::OnPlayerSpawnEnd(*player, m_objectId);
	}

	// Called when a weapon is created
	void __stdcall OnObjectCreation(ident m_objectId)
	{
		scripting::events::OnObjectCreation(m_objectId);
	}

	bool __stdcall OnObjectCreationAttempt(objects::s_object_creation_disposition* creation_info)
	{
		/*! \todo make sure the player is correct */
		return scripting::events::OnObjectCreationAttempt(creation_info);
	}
	/*! \todo lookuptag : tag -> memory address
	 *		  gettagid: tag -> tag (map) id
	 */
	ident __stdcall OnWeaponAssignment(DWORD playerId, ident owningObjectId,
		s_object_info* curWeapon, DWORD order)
	{
		halo::s_player* player = game::GetPlayer(playerId);
		ident weap_id = curWeapon->id, result_id;

		bool b = scripting::events::OnWeaponAssignment(player, owningObjectId, order, weap_id,
			result_id);

		if (!b) return weap_id;
		return result_id;
	}
	
	/*! \todo make phasor go through a script to detect which functions
	 *! it has when loading, then blacklist the rest of the expected ones. */
	void OnClientUpdate(s_player& player)
	{
		// scripts called from server
		player.afk->CheckPlayerActivity();
	}
}}