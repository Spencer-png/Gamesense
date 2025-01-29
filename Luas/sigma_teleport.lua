local bit_lshift = bit.lshift
local FLAG_VISIBLE = bit_lshift(1, 11)
local DELAY_TIME = 0.3
local weapons = require("gamesense/csgo_weapons")
local screen_size = {client.screen_size()}
local screen_center = {
    screen_size[1] / 2,
    screen_size[2] / 2
}

local teleport_key = ui.new_hotkey("AA", "Other", "Far Teleport")
local teleport_hittable = ui.new_hotkey("AA", "Other", "Far Teleport When Hittable")
local dt_ref, dt_hotkey, dt_mode = ui.reference("RAGE", "Aimbot", "Double tap")
local quickpeek_ref, quickpeek_hotkey = ui.reference("RAGE", "Other", "Quick peek assist")

local g_command_number = nil
local g_tick_base_old = nil
local g_tick_diff = nil
local g_teleport_active = false
local g_maximize_teleport = false

local function reset_state()
    g_tick_base_old = nil
    g_tick_diff = nil
end

client.set_event_callback("run_command", function(cmd)
    g_command_number = cmd.command_number
end)

client.set_event_callback("predict_command", function(cmd)
    if cmd.command_number == g_command_number then
        g_command_number = nil
        local tick_base = entity.get_prop(entity.get_local_player(), "m_nTickBase")
        
        if g_tick_base_old ~= nil then
            g_tick_diff = tick_base - g_tick_base_old
        end
        
        g_tick_base_old = math.max(tick_base, g_tick_base_old or 0)
    end
end)

client.set_event_callback("setup_command", function(cmd)
    if not ui.get(dt_ref) or not ui.get(dt_hotkey) or ui.get(dt_mode) ~= "Defensive" then
        g_maximize_teleport = false
        g_teleport_active = false
        return
    end

    local is_moving = cmd.in_forward == 1 or cmd.in_back == 1 or 
                     cmd.in_moveleft == 1 or cmd.in_moveright == 1 or 
                     cmd.in_jump == 1

    if not g_teleport_active and is_moving then
        g_teleport_active = ui.get(teleport_key)

        if not g_teleport_active and ui.get(teleport_hittable) then
            for _, player in ipairs(entity.get_players(true)) do
                if bit.band(entity.get_esp_data(player).flags or 0, FLAG_VISIBLE) ~= 0 then
                    g_teleport_active = true
                    break
                end
            end
        end
    end

    if g_teleport_active then
        cmd.force_defensive = true

        if g_tick_diff >= 14 then
            g_maximize_teleport = true
        end

        local weapon = entity.get_player_weapon(entity.get_local_player())
        if (g_maximize_teleport and g_tick_diff == 0) or (weapon and weapons[entity.get_prop(weapon, "m_iItemDefinitionIndex")].type == "grenade") then
            ui.set(dt_ref, false)
            client.delay_call(DELAY_TIME, ui.set, dt_ref, true)
            g_teleport_active = false
            g_maximize_teleport = false
        end
    end
end)

client.set_event_callback("paint", function()
    if g_maximize_teleport then
        renderer.indicator(143, 194, 21, 255, "+/- MAXIMIZING TELEPORT DISTANCE")
        renderer.text(screen_center[1] - 60, screen_center[2] + 40, 143, 207, 219, 255, "-c", 0, "+/- MAXIMIZING TELEPORT DISTANCE")
    elseif ui.get(dt_ref) or ui.get(quickpeek_ref) then
        local text = "TELEPORT READY"
        if g_teleport_active then
            renderer.indicator(255, 0, 50, 255, text)
        else
            renderer.indicator(255, 255, 255, 255, text)
        end
    end
end)