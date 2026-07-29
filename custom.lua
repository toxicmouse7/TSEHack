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
	local box_transform, box_extents = Unit.box(player.avatar_unit)
	local box_center = Matrix4x4.translation(box_transform)
	local box_center_screen = camera:world_to_screen(box_center)
	local box_x_axis = Matrix4x4.x(box_transform)
	local box_y_axis = Matrix4x4.y(box_transform)
	local box_z_axis = Matrix4x4.z(box_transform)
	local min_x, min_y = math.huge, math.huge
	local max_x, max_y = -math.huge, -math.huge

	for _, x_sign in ipairs({ -1, 1 }) do
		for _, y_sign in ipairs({ -1, 1 }) do
			for _, z_sign in ipairs({ -1, 1 }) do
				local corner = box_center
					+ box_x_axis * (box_extents.x * x_sign)
					+ box_y_axis * (box_extents.y * y_sign)
					+ box_z_axis * (box_extents.z * z_sign)
				local corner_screen = camera:world_to_screen(corner)
				min_x = math.min(min_x, corner_screen.x)
				min_y = math.min(min_y, corner_screen.z)
				max_x = math.max(max_x, corner_screen.x)
				max_y = math.max(max_y, corner_screen.z)
			end
		end
	end

	return box_center_screen, min_x, min_y, max_x, max_y
end

if get_current_world_name() == "game_world" then
	local players = get_players()
	local _, res_y = Application.resolution()
	local local_player = players[Network.peer_id()]
	local camera_state = EntityAux.state(local_player.avatar_unit, "camera_target")
	local local_camera = get_player_camera(camera_state.viewport_name)
	local result = {}

	for peer_id, player in pairs(players) do
		local damage_receiver = EntityAux.state(player.avatar_unit, "damage_receiver_ext")
		if peer_id ~= Network.peer_id() and damage_receiver ~= nil and not damage_receiver.dead then
			local center, left, top, right, bottom = get_player_screen_position(local_camera, player)
			result[peer_id] = {
				x = center.x,
				y = res_y - center.z,
				left = left,
				top = res_y - bottom,
				right = right,
				bottom = res_y - top
			}
		end
	end

	return result
end

return 0
