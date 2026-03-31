local pk2 = require("pk2")

local forestLib = require("lib_forest")
local forestLib2 = require("lib_forest2")
local spiderLib = require("lib_spider")

local trolledRooster = pk2.loadSpritePrototype("trolled_rooster.spr2")
local trolledRoosterNaked = pk2.loadSpritePrototype("trolled_r_naked.spr2")

local trollConv1 = pk2.loadSpritePrototype("spr_troll_conv1.spr2")
local trollConv2 = pk2.loadSpritePrototype("spr_troll_conv2.spr2")

local trollFriendly = pk2.loadSpritePrototype("troll4.spr2")

local rooster = pk2.loadSpritePrototype("rooster.spr2")
local r_naked = pk2.loadSpritePrototype("r_naked.spr2")


pk2.addCommand("spiderBounce", spiderLib.bounce)

pk2.addCommand("rodentNest", forestLib2.rodentNest)
pk2.addCommand("forestBunny", forestLib2.forestBunny)
pk2.addCommand("climbingRodent", forestLib2.climbingRodent)


pk2.events.addListener(pk2.events.GAME_STARTED, function ()
    --forestLib.addTree(pk2.getSector(0), 62*32,  15*32, math.random(0, 6)==0)
    --forestLib.addFungus(16*32, 23*32)
    forestLib.genForest(pk2.getSector(0), 170, 426, 68)
end )


pk2.events.addListener(pk2.events.GAME_TICK, function ()
    forestLib2.updateForest()
end)


pk2.events.addListener(pk2.events.EVENT2, function ()
    local py
    if math.random(0, 3) == 0 then
        py = "import matplotlib.pyplot as plt"
    else
        py = "import numpy as np"
    end

    pk2.showInfo(py)
end)



-- It is safe as they are prototypes not sprites
local proto_ammo1_bk
local proto_ammo2_bk

pk2.addCommand("trolled", function (troll)

    local player = pk2.getPlayerIfAccessible()

    if player~=nil and player.prototype~=trolledRooster and player.prototype~=trolledRoosterNaked and math.abs(player.x - troll.x) < 224 then

        -- It is safe as they are prototypes not sprites
        proto_ammo1_bk = player.ammo1
        proto_ammo2_bk = player.ammo2
        
        if player.energy > 1 then
            player:transformTo(trolledRooster)
        else
            player:transformTo(trolledRoosterNaked)
        end

        --player.ammo1 = proto_ammo1_bk
        --player.ammo2 = proto_ammo2_bk


        pk2.addSprite(trollConv1, troll.x, troll.y - 120, troll)
        return true
    end


    return false
end)

pk2.addCommand("trolled2", function (troll)
    pk2.startTrolling()
    return true
end)


pk2.addCommand("trolled3", function (troll)
    local player = pk2.getPlayerIfAccessible()
    if player==nil then
        return false
    end

    if not pk2.isTrolled() then
        troll:transformTo(trollFriendly)
        troll.enemy = false

        pk2.addSprite(trollConv2, troll.x, troll.y - 120, troll)
        
        if player.energy > 1 then
            player:transformTo(rooster)
        else
            player:transformTo(r_naked)
        end

        player.ammo1 = proto_ammo1_bk
        player.ammo2 = proto_ammo2_bk

        return true
    end

    local targetX

    if player.x < troll.x then
        targetX = troll.origX - 90
    else
        targetX = troll.origX + 90
    end

    if troll:flyToWaypointX(targetX) then
        troll.a = 0
        if troll.flipX then
            troll.x = troll.x - 45
        else
            troll.x = troll.x + 45
        end

        return true
    end


    return false
end)

pk2.addCommand("trolled4", function (troll)
    local player = pk2.getPlayerIfAccessible()
    if player~=nil then
        player:die()
        return true
    end

    return false
end)