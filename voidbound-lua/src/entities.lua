local entities = {}

local next_id = 1

local function id(prefix)
    local value = prefix .. tostring(next_id)
    next_id = next_id + 1
    return value
end

function entities.player(x, y)
    return {
        id = "player",
        kind = "player",
        x = x,
        y = y,
        hp = 28,
        max_hp = 28,
        ap = 3,
        max_ap = 3,
        armor = 0,
        score = 0,
        gold = 0,
        xp = 0,
        level = 1,
        potions = 1,
        cooldowns = { dash = 0, blast = 0, shield = 0 },
        alive = true,
    }
end

function entities.enemy(kind, x, y, wave)
    local templates = {
        stalker = { hp = 8 + wave, damage = 3, ap = 2, glyph = "s" },
        brute = { hp = 14 + wave * 2, damage = 5, ap = 1, glyph = "B" },
        wisp = { hp = 5 + wave, damage = 2, ap = 3, glyph = "w" },
    }
    local t = templates[kind] or templates.stalker
    return {
        id = id("enemy-"),
        kind = kind,
        x = x,
        y = y,
        hp = t.hp,
        max_hp = t.hp,
        damage = t.damage,
        ap = t.ap,
        max_ap = t.ap,
        glyph = t.glyph,
        alive = true,
    }
end

function entities.distance(a, b)
    return math.abs(a.x - b.x) + math.abs(a.y - b.y)
end

function entities.damage(target, amount)
    local blocked = math.min(target.armor or 0, amount)
    target.armor = (target.armor or 0) - blocked
    local final = amount - blocked
    target.hp = target.hp - final
    if target.hp <= 0 then
        target.hp = 0
        target.alive = false
    end
    return final
end

function entities.heal(target, amount)
    target.hp = math.min(target.max_hp, target.hp + amount)
end

function entities.gain_xp(player, amount)
    player.xp = player.xp + amount
    local needed = player.level * 18
    local leveled = false
    while player.xp >= needed do
        player.xp = player.xp - needed
        player.level = player.level + 1
        player.max_hp = player.max_hp + 4
        player.hp = player.max_hp
        if player.level % 2 == 0 then
            player.max_ap = player.max_ap + 1
        end
        player.ap = player.max_ap
        leveled = true
        needed = player.level * 18
    end
    return leveled
end

function entities.reset_turn(unit)
    unit.ap = unit.max_ap
    if unit.cooldowns then
        for key, value in pairs(unit.cooldowns) do
            unit.cooldowns[key] = math.max(0, value - 1)
        end
    end
end

return entities
