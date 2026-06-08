local entities = require("entities")
local map = require("map")

local ai = {}

local steps = {
    { 1, 0 },
    { -1, 0 },
    { 0, 1 },
    { 0, -1 },
}

local function occupied(enemies, self, x, y)
    for _, enemy in ipairs(enemies) do
        if enemy ~= self and enemy.alive and enemy.x == x and enemy.y == y then
            return true
        end
    end
    return false
end

function ai.take_turn(game, enemy)
    enemy.ap = enemy.max_ap
    while enemy.ap > 0 and enemy.alive and game.player.alive do
        if entities.distance(enemy, game.player) == 1 then
            enemy.ap = enemy.ap - 1
            entities.damage(game.player, enemy.damage)
        else
            local best = nil
            local best_dist = math.huge
            for _, step in ipairs(steps) do
                local nx = enemy.x + step[1]
                local ny = enemy.y + step[2]
                local is_player_tile = nx == game.player.x and ny == game.player.y
                if map.is_walkable(game.arena, nx, ny) and not occupied(game.enemies, enemy, nx, ny) and not is_player_tile then
                    local dist = math.abs(nx - game.player.x) + math.abs(ny - game.player.y)
                    if dist < best_dist then
                        best = { x = nx, y = ny }
                        best_dist = dist
                    end
                end
            end
            if not best then
                enemy.ap = 0
            else
                enemy.x = best.x
                enemy.y = best.y
                enemy.ap = enemy.ap - 1
            end
        end
    end
end

return ai
