#pragma once

#include <string>

const std::string lua_script = R"(
local function table_to_string_top_level(t)
    local result = {}
    for key, value in pairs(t) do
        table.insert(result, string.format("%s: %s", tostring(key), tostring(value)))
    end
    return table.concat(result, "\n")
end

local function get_players()
	return Boot.machine._state.player_manager.players
end

local function get_current_world_name()
	return Boot.machine._state.world_name
end

local function get_player_camera(viewport_name)
	return Boot.machine._state.gui_manager.world_manager:camera(viewport_name)
end

local function get_player_screen_position(camera, player)
	local player_world_position = UnitAux.world_position(player.avatar_unit)

	return camera:world_to_screen(player_world_position)
end

if (get_current_world_name() == "game_world") then
	local players = get_players()
	local res_x, res_y = Application.resolution()
	local local_player = players[Network.peer_id()]

	local camera_state = EntityAux.state(local_player.avatar_unit, "camera_target")
	local viewport_name = camera_state.viewport_name
	local local_camera = get_player_camera(viewport_name)

	local result = {}

	for peer_id, player in pairs(players) do
		local damage_receiver = EntityAux.state(unit, "damage_receiver_ext")
		if peer_id ~= Network.peer_id() and damage_receiver ~= nil and not damage_receiver.dead then
			local player_screen_position = get_player_screen_position(local_camera, player)
			result[peer_id] = {}
			result[peer_id].x = player_screen_position.x
			result[peer_id].y = res_y - player_screen_position.z - 15
		end
	end

	return result
end

return 0
)";