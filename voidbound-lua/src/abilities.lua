local entities = require("entities")
local map = require("map")

local abilities = {}

local directions = {
    up = { 0, -1 },
    down = { 0, 1 },
    left = { -1, 0 },
    right = { 1, 0 },
}

local function enemy_at(enemies, x, y)
    for _, enemy in ipairs(enemies) do
        if enemy.alive and enemy.x == x and enemy.y == y then
            return enemy
        end
    end
    return nil
end

local function spend(player, cost)
    if player.ap < cost then
        return false, "not enough action points"
    end
    player.ap = player.ap - cost
    return true
end

local function reward(game, target)
    if target.alive then
        return ""
    end
    local gold = target.kind == "brute" and 5 or 3
    local xp = target.kind == "brute" and 10 or 7
    game.player.score = game.player.score + 10 + gold
    game.player.gold = game.player.gold + gold
    local leveled = entities.gain_xp(game.player, xp)
    return leveled and " defeated " .. target.kind .. " and leveled up" or " defeated " .. target.kind
end

function abilities.move(game, dir)
    local delta = directions[dir]
    if not delta then
        return false, "unknown direction"
    end
    local ok, err = spend(game.player, 1)
    if not ok then
        return false, err
    end
    local nx = game.player.x + delta[1]
    local ny = game.player.y + delta[2]
    if not map.is_walkable(game.arena, nx, ny) or enemy_at(game.enemies, nx, ny) then
        game.player.ap = game.player.ap + 1
        return false, "blocked"
    end
    game.player.x = nx
    game.player.y = ny
    return true
end

function abilities.strike(game, dir)
    local delta = directions[dir]
    if not delta then
        return false, "unknown direction"
    end
    local ok, err = spend(game.player, 1)
    if not ok then
        return false, err
    end
    local target = enemy_at(game.enemies, game.player.x + delta[1], game.player.y + delta[2])
    if not target then
        return false, "no target"
    end
    local dealt = entities.damage(target, 6)
    return true, "dealt " .. dealt .. reward(game, target)
end

function abilities.dash(game, dir)
    if game.player.cooldowns.dash > 0 then
        return false, "dash is on cooldown"
    end
    local delta = directions[dir]
    if not delta then
        return false, "unknown direction"
    end
    local ok, err = spend(game.player, 2)
    if not ok then
        return false, err
    end
    for _ = 1, 3 do
        local nx = game.player.x + delta[1]
        local ny = game.player.y + delta[2]
        if not map.is_walkable(game.arena, nx, ny) or enemy_at(game.enemies, nx, ny) then
            break
        end
        game.player.x = nx
        game.player.y = ny
    end
    game.player.cooldowns.dash = 3
    return true
end

function abilities.blast(game)
    if game.player.cooldowns.blast > 0 then
        return false, "blast is on cooldown"
    end
    local ok, err = spend(game.player, 2)
    if not ok then
        return false, err
    end
    local hits = 0
    for _, enemy in ipairs(game.enemies) do
        if enemy.alive and entities.distance(game.player, enemy) <= 2 then
            entities.damage(enemy, 4)
            hits = hits + 1
            reward(game, enemy)
        end
    end
    game.player.cooldowns.blast = 4
    return true, "hit " .. hits
end

function abilities.shield(game)
    if game.player.cooldowns.shield > 0 then
        return false, "shield is on cooldown"
    end
    local ok, err = spend(game.player, 1)
    if not ok then
        return false, err
    end
    game.player.armor = game.player.armor + 6
    game.player.cooldowns.shield = 4
    return true, "shielded"
end

function abilities.potion(game)
    if game.player.potions <= 0 then
        return false, "no potions"
    end
    if game.player.hp >= game.player.max_hp then
        return false, "already full hp"
    end
    game.player.potions = game.player.potions - 1
    entities.heal(game.player, 12)
    return true, "used potion"
end

return abilities
