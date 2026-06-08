local abilities = require("abilities")
local ai = require("ai")
local entities = require("entities")
local map = require("map")

local game = {}

local enemy_cycle = { "stalker", "wisp", "brute" }

local function alive_enemies(enemies)
    local count = 0
    for _, enemy in ipairs(enemies) do
        if enemy.alive then
            count = count + 1
        end
    end
    return count
end

function game.new(seed)
    local arena = map.create(18, 12, seed or 1337)
    local player = entities.player(2, 2)
    local state = {
        arena = arena,
        player = player,
        enemies = {},
        wave = 0,
        turn = 1,
        log = {},
        over = false,
        victory = false,
    }
    game.spawn_wave(state)
    return state
end

function game.spawn_wave(state)
    state.wave = state.wave + 1
    local used = {}
    for i = 1, math.min(2 + state.wave, 6) do
        local spawn = map.find_spawn(state.arena, state.player.x + i, state.player.y)
        while used[spawn.y .. ":" .. spawn.x] or not map.is_walkable(state.arena, spawn.x, spawn.y) do
            spawn.x = spawn.x - 1
            if spawn.x < 2 then
                spawn.x = state.arena.width - 2
                spawn.y = spawn.y - 1
            end
            if spawn.y < 2 then
                spawn.y = state.arena.height - 2
            end
        end
        used[spawn.y .. ":" .. spawn.x] = true
        local kind = enemy_cycle[((i + state.wave - 2) % #enemy_cycle) + 1]
        state.enemies[#state.enemies + 1] = entities.enemy(kind, spawn.x, spawn.y, state.wave)
    end
    state.log[#state.log + 1] = "wave " .. state.wave .. " entered the arena"
end

function game.cleanup(state)
    local kept = {}
    for _, enemy in ipairs(state.enemies) do
        if enemy.alive then
            kept[#kept + 1] = enemy
        end
    end
    state.enemies = kept
end

function game.apply_tile(state)
    local tile = map.tile(state.arena, state.player.x, state.player.y)
    if tile == "^" then
        entities.damage(state.player, 2)
        state.log[#state.log + 1] = "hazard dealt 2 damage"
    elseif tile == "+" then
        entities.heal(state.player, 4)
        state.arena.grid[state.player.y][state.player.x] = "."
        state.log[#state.log + 1] = "recovered 4 hp"
    elseif tile == ">" and state.wave >= 3 and alive_enemies(state.enemies) == 0 then
        state.over = true
        state.victory = true
    end
end

function game.player_action(state, command, arg)
    if state.over then
        return false, "game is over"
    end
    local ok, message
    if command == "move" then
        ok, message = abilities.move(state, arg)
    elseif command == "strike" then
        ok, message = abilities.strike(state, arg)
    elseif command == "dash" then
        ok, message = abilities.dash(state, arg)
    elseif command == "blast" then
        ok, message = abilities.blast(state)
    elseif command == "shield" then
        ok, message = abilities.shield(state)
    elseif command == "wait" then
        state.player.ap = 0
        ok, message = true, "wait"
    else
        return false, "unknown command"
    end
    if ok then
        game.cleanup(state)
        game.apply_tile(state)
        if state.player.ap <= 0 then
            game.end_player_turn(state)
        end
    end
    return ok, message
end

function game.end_player_turn(state)
    for _, enemy in ipairs(state.enemies) do
        ai.take_turn(state, enemy)
    end
    if not state.player.alive then
        state.over = true
        state.victory = false
        return
    end
    game.cleanup(state)
    if alive_enemies(state.enemies) == 0 then
        if state.wave >= 3 then
            state.log[#state.log + 1] = "portal opened"
        else
            game.spawn_wave(state)
        end
    end
    entities.reset_turn(state.player)
    state.turn = state.turn + 1
end

function game.summary(state)
    return string.format(
        "turn=%d wave=%d hp=%d/%d armor=%d ap=%d score=%d enemies=%d",
        state.turn,
        state.wave,
        state.player.hp,
        state.player.max_hp,
        state.player.armor,
        state.player.ap,
        state.player.score,
        alive_enemies(state.enemies)
    )
end

function game.render(state)
    return map.render(state.arena, state.player, state.enemies) .. "\n" .. game.summary(state)
end

return game
