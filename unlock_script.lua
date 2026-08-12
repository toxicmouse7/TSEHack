local function add_all_items(state)
	state.backpack = state.backpack or {}
	for key, _ in pairs(KeyToItemLookup) do
		if not string.find(key, "^henchmen_") then
			state.backpack[key] = true
		end
	end
end

local function install_ownership_hook(class_table, original)
	class_table.on_ownership_changed = function(self, ...)
		add_all_items(self)
		return original(self, ...)
	end
end

if rawget(_G, "SHOWDOWN_LOCAL_UNLOCK_INSTALLED") then
	return 1
end

if not KeyToItemLookup or not GameStateMenu or not GameStateInGame then
	return 0
end

local original_menu_ownership = GameStateMenu.on_ownership_changed
local original_ingame_ownership = GameStateInGame.on_ownership_changed
if type(original_menu_ownership) ~= "function" or type(original_ingame_ownership) ~= "function" then
	return 0
end

install_ownership_hook(GameStateMenu, original_menu_ownership)
install_ownership_hook(GameStateInGame, original_ingame_ownership)

OverrideStatus = true
if Backpack then
	Backpack.needs_update = true
end
rawset(_G, "SHOWDOWN_LOCAL_UNLOCK_INSTALLED", true)
return 1
