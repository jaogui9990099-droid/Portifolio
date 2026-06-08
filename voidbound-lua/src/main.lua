package.path = package.path .. ";src/?.lua"

local game = require("game")

local seed = tonumber(arg[1]) or 1337
local state = game.new(seed)

print("Voidbound Arena")
print("Commands: move <up|down|left|right>, strike <dir>, dash <dir>, blast, shield, wait, quit")

while not state.over do
    print("")
    print(game.render(state))
    io.write("> ")
    local line = io.read("*line")
    if not line or line == "quit" then
        break
    end
    local command, value = line:match("^(%S+)%s*(%S*)")
    local ok, message = game.player_action(state, command, value ~= "" and value or nil)
    if not ok then
        print("error: " .. tostring(message))
    elseif message then
        print(message)
    end
end

if state.over and state.victory then
    print("Victory. Final score: " .. state.player.score)
elseif state.over then
    print("Defeat. Final score: " .. state.player.score)
else
    print("Run abandoned.")
end
