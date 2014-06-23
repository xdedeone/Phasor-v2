#include "../phasor-lua.hpp"
#include "../../Phasor/Halo/Player.h"
#include "../../Phasor/Halo/Game/Game.h"
#include "../../Phasor/Admin.h"

int l_resolveplayer(lua_State* L) {
	halo::s_player* player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
	return phlua::callback::pushReturns(L, std::make_tuple(player->mem->playerNum+1));
}

int l_rresolveplayer(lua_State* L) {
	size_t machineId;
	std::tie(machineId) = phlua::callback::getArguments<size_t>(L, __FUNCTION__);
	halo::s_player* player = halo::game::getPlayerFromRconId(machineId);

	if (player)
		return phlua::callback::pushReturns(L, std::make_tuple(player->memory_id));
	else
		return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
}

int l_getplayer(lua_State* L) {
	boost::optional<halo::s_player*> player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
	
	if (player)
		return phlua::callback::pushReturns(L, std::make_tuple((*player)->memory_id));
	else
		return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
}

int l_getip(lua_State* L) {
	halo::s_player* player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
	return phlua::callback::pushReturns(L, std::make_tuple(player->ip));
}

int l_getport(lua_State* L) {
	halo::s_player* player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
	return phlua::callback::pushReturns(L, std::make_tuple(player->port));
}

int l_getname(lua_State* L) {
	halo::s_player* player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
	return phlua::callback::pushReturns(L, std::make_tuple(player->mem->team));
}

int l_gethash(lua_State* L) {
	halo::s_player* player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
	return phlua::callback::pushReturns(L, std::make_tuple(player->mem->playerName));
}

int l_getteamsize(lua_State* L) {
	unsigned char team;
	std::tie(team) = phlua::callback::getArguments<size_t>(L, __FUNCTION__);
	
	unsigned char count = 0;
	for (int i = 0; i < 16; i++) {
		halo::s_player* player = halo::game::getPlayer(i);
		if (player && player->mem->team == team) count++;
	}
	return phlua::callback::pushReturns(L, std::make_tuple(count));
}

int l_getplayerobjectid(lua_State* L) {
	boost::optional<halo::s_player*> player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);

	if (player && (*player)->mem->object_id.valid())
		return phlua::callback::pushReturns(L, std::make_tuple((*player)->mem->object_id));
	else
		return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
}

int l_isadmin(lua_State* L) {
	halo::s_player* player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);
	return phlua::callback::pushReturns(L, std::make_tuple(player->is_admin));
}

int l_getadminlvl(lua_State* L) {
	halo::s_player* player;
	std::tie(player) = phlua::callback::getArguments<decltype(player)>(L, __FUNCTION__);

	int level;
	if (Admin::getLevel(player->hash, &level)) {
		return phlua::callback::pushReturns(L, std::make_tuple(level));
	} else {
		return phlua::callback::pushReturns(L, std::make_tuple(lua::types::Nil()));
	}
}

int l_setadmin(lua_State* L) {
	halo::s_player* player;
	phlua::callback::getArguments<>(L, __FUNCTION__);
	player->is_admin = true;
	return 0;
}