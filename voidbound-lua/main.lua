package.path = package.path .. ";src/?.lua"

local game = require("game")

local state
local tile = 42
local ox = 42
local oy = 82
local message = "WASD move | Arrow keys strike | Space blast | Shift dash | E shield | Enter wait | R restart"

local colors = {
    bg = { 0.05, 0.06, 0.08 },
    panel = { 0.10, 0.12, 0.16 },
    floor = { 0.16, 0.18, 0.23 },
    wall = { 0.35, 0.38, 0.46 },
    hazard = { 0.76, 0.24, 0.32 },
    heal = { 0.22, 0.65, 0.40 },
    portal = { 0.42, 0.32, 0.82 },
    player = { 0.35, 0.62, 1.00 },
    enemy = { 1.00, 0.38, 0.30 },
    enemy2 = { 1.00, 0.75, 0.25 },
    text = { 0.90, 0.93, 0.96 },
    muted = { 0.58, 0.64, 0.70 },
}

local function restart()
    state = game.new(os.time() % 100000)
    message = "New run started."
end

function love.load()
    love.graphics.setDefaultFilter("nearest", "nearest")
    restart()
end

local function draw_tile(x, y, kind)
    local px = ox + (x - 1) * tile
    local py = oy + (y - 1) * tile
    local c = colors.floor
    if kind == "#" then c = colors.wall end
    if kind == "^" then c = colors.hazard end
    if kind == "+" then c = colors.heal end
    if kind == ">" then c = colors.portal end
    love.graphics.setColor(c)
    love.graphics.rectangle("fill", px, py, tile - 3, tile - 3, 6, 6)
    if kind == "^" then
        love.graphics.setColor(0.12, 0.05, 0.07)
        love.graphics.polygon("fill", px + tile / 2, py + 8, px + tile - 9, py + tile - 10, px + 9, py + tile - 10)
    elseif kind == "+" then
        love.graphics.setColor(0.08, 0.18, 0.12)
        love.graphics.rectangle("fill", px + 16, py + 8, 8, tile - 20, 3, 3)
        love.graphics.rectangle("fill", px + 8, py + 16, tile - 20, 8, 3, 3)
    elseif kind == ">" then
        love.graphics.setColor(0.75, 0.65, 1.0)
        love.graphics.circle("line", px + tile / 2, py + tile / 2, tile / 3)
    end
end

local function draw_unit(unit, color, label)
    if not unit.alive then return end
    local px = ox + (unit.x - 1) * tile
    local py = oy + (unit.y - 1) * tile
    love.graphics.setColor(color)
    love.graphics.circle("fill", px + tile / 2 - 1, py + tile / 2 - 1, tile * 0.34)
    love.graphics.setColor(0.04, 0.05, 0.07)
    love.graphics.print(label, px + tile / 2 - 5, py + tile / 2 - 8)
end

local function direction_from_key(key)
    if key == "up" or key == "w" then return "up" end
    if key == "down" or key == "s" then return "down" end
    if key == "left" or key == "a" then return "left" end
    if key == "right" or key == "d" then return "right" end
    return nil
end

local function action(command, arg)
    local ok, msg = game.player_action(state, command, arg)
    message = ok and (msg or command) or ("Blocked: " .. tostring(msg))
end

function love.keypressed(key)
    if key == "r" then restart(); return end
    if key == "escape" then love.event.quit(); return end
    if state.over then return end

    if key == "space" then action("blast"); return end
    if key == "e" then action("shield"); return end
    if key == "return" then action("wait"); return end

    local dir = direction_from_key(key)
    if not dir then return end

    if love.keyboard.isDown("lshift") or love.keyboard.isDown("rshift") then
        action("dash", dir)
    elseif key == "up" or key == "down" or key == "left" or key == "right" then
        action("strike", dir)
    else
        action("move", dir)
    end
end

function love.draw()
    love.graphics.clear(colors.bg)
    love.graphics.setColor(colors.text)
    love.graphics.setFont(love.graphics.newFont(24))
    love.graphics.print("Voidbound Arena", 42, 28)
    love.graphics.setFont(love.graphics.newFont(14))
    love.graphics.setColor(colors.muted)
    love.graphics.print(message, 42, 58)

    for y = 1, state.arena.height do
        for x = 1, state.arena.width do
            draw_tile(x, y, state.arena.grid[y][x])
        end
    end

    for _, enemy in ipairs(state.enemies) do
        local color = enemy.kind == "brute" and colors.enemy2 or colors.enemy
        draw_unit(enemy, color, enemy.glyph:upper())
    end
    draw_unit(state.player, colors.player, "@")

    local panel_x = ox + state.arena.width * tile + 28
    love.graphics.setColor(colors.panel)
    love.graphics.rectangle("fill", panel_x, oy, 270, 300, 10, 10)
    love.graphics.setColor(colors.text)
    love.graphics.setFont(love.graphics.newFont(16))
    love.graphics.print("Run Status", panel_x + 18, oy + 18)
    love.graphics.setFont(love.graphics.newFont(13))
    local lines = {
        "Turn: " .. state.turn,
        "Wave: " .. state.wave,
        "HP: " .. state.player.hp .. "/" .. state.player.max_hp,
        "Armor: " .. state.player.armor,
        "AP: " .. state.player.ap .. "/" .. state.player.max_ap,
        "Score: " .. state.player.score,
        "Dash CD: " .. state.player.cooldowns.dash,
        "Blast CD: " .. state.player.cooldowns.blast,
        "Shield CD: " .. state.player.cooldowns.shield,
    }
    for i, line in ipairs(lines) do
        love.graphics.print(line, panel_x + 18, oy + 48 + i * 23)
    end

    if state.over then
        love.graphics.setColor(0, 0, 0, 0.72)
        love.graphics.rectangle("fill", 0, 0, love.graphics.getWidth(), love.graphics.getHeight())
        love.graphics.setColor(colors.text)
        love.graphics.setFont(love.graphics.newFont(34))
        love.graphics.printf(state.victory and "Victory" or "Defeat", 0, 300, love.graphics.getWidth(), "center")
        love.graphics.setFont(love.graphics.newFont(16))
        love.graphics.printf("Press R to restart", 0, 346, love.graphics.getWidth(), "center")
    end
end
