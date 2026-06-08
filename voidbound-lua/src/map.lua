local map = {}

local function rng(seed)
    local state = seed or 1337
    return function(min, max)
        state = (state * 1103515245 + 12345) % 2147483648
        local value = state / 2147483648
        if min and max then
            return min + math.floor(value * (max - min + 1))
        end
        return value
    end
end

function map.create(width, height, seed)
    local rand = rng(seed)
    local grid = {}
    for y = 1, height do
        grid[y] = {}
        for x = 1, width do
            local edge = x == 1 or y == 1 or x == width or y == height
            local roll = rand()
            if edge then
                grid[y][x] = "#"
            elseif roll < 0.10 then
                grid[y][x] = "#"
            elseif roll < 0.15 then
                grid[y][x] = "^"
            elseif roll < 0.18 then
                grid[y][x] = "+"
            elseif roll < 0.205 then
                grid[y][x] = "$"
            elseif roll < 0.225 then
                grid[y][x] = "?"
            else
                grid[y][x] = "."
            end
        end
    end
    grid[2][2] = "."
    grid[height - 1][width - 1] = ">"
    return { width = width, height = height, grid = grid, seed = seed or 1337 }
end

function map.tile(arena, x, y)
    if x < 1 or y < 1 or x > arena.width or y > arena.height then
        return "#"
    end
    return arena.grid[y][x]
end

function map.is_walkable(arena, x, y)
    return map.tile(arena, x, y) ~= "#"
end

function map.find_spawn(arena, from_x, from_y)
    local best = nil
    local best_dist = -1
    for y = 2, arena.height - 1 do
        for x = 2, arena.width - 1 do
            if map.is_walkable(arena, x, y) then
                local dist = math.abs(x - from_x) + math.abs(y - from_y)
                if dist > best_dist then
                    best = { x = x, y = y }
                    best_dist = dist
                end
            end
        end
    end
    return best or { x = arena.width - 2, y = arena.height - 2 }
end

function map.render(arena, player, enemies)
    local rows = {}
    local occupied = {}
    occupied[player.y .. ":" .. player.x] = "@"
    for _, enemy in ipairs(enemies) do
        if enemy.alive then
            occupied[enemy.y .. ":" .. enemy.x] = enemy.glyph
        end
    end
    for y = 1, arena.height do
        local line = {}
        for x = 1, arena.width do
            line[#line + 1] = occupied[y .. ":" .. x] or arena.grid[y][x]
        end
        rows[#rows + 1] = table.concat(line)
    end
    return table.concat(rows, "\n")
end

return map
