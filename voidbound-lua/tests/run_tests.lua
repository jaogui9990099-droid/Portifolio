package.path = package.path .. ";src/?.lua;../src/?.lua"

local game = require("game")

local failures = 0

local function assert_true(name, value)
    if not value then
        failures = failures + 1
        io.stderr:write("FAIL: " .. name .. "\n")
    else
        print("PASS: " .. name)
    end
end

local function test_game_starts_with_player_and_enemies()
    local state = game.new(42)
    assert_true("player alive on start", state.player.alive)
    assert_true("starts with enemies", #state.enemies > 0)
    assert_true("starts on wave one", state.wave == 1)
end

local function test_player_can_spend_action_points()
    local state = game.new(42)
    local before = state.player.ap
    local ok = game.player_action(state, "shield")
    assert_true("shield action succeeds", ok)
    assert_true("shield spends ap", state.player.ap < before)
    assert_true("shield adds armor", state.player.armor > 0)
end

local function test_blast_damages_nearby_enemy()
    local state = game.new(42)
    state.enemies = {
        { id = "test", kind = "stalker", x = state.player.x + 1, y = state.player.y, hp = 5, max_hp = 5, damage = 1, ap = 1, max_ap = 1, glyph = "s", alive = true }
    }
    local ok = game.player_action(state, "blast")
    assert_true("blast action succeeds", ok)
    assert_true("blast damages enemy", state.enemies[1] == nil or state.enemies[1].hp < 5)
end

test_game_starts_with_player_and_enemies()
test_player_can_spend_action_points()
test_blast_damages_nearby_enemy()

if failures > 0 then
    os.exit(1)
end

print("All Lua gameplay tests passed.")
