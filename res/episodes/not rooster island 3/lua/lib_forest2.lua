local forestlib2 = {}

local pk2 = require("pk2")
local protoForestBunny = pk2.loadSpritePrototype("bunny.spr2")

local protoRodentWalking = pk2.loadSpritePrototype("rodent1.spr2")
local protoRodentClimbing = pk2.loadSpritePrototype("rodent2.spr2")

forestlib2.startX = 161 * 32
forestlib2.stopX = 433 * 32
forestlib2.startY = 49 * 32
forestlib2.stopY = 70 * 32



function forestlib2.climbingRodent(rodent)

    rodent.b = -2
    local tree = rodent.target
    if tree~=nil and rodent.y <= tree.y - 130 then
        rodent:transform()
        rodent.jumpTimer = 1

        local player = pk2.getPlayerIfAccessible()
        rodent.flipX = player~=nil and player.x < rodent.x


        return true
    end

    return false
end

function forestlib2.rodentNest(tree)
    local nest = tree.target
    if nest~=nil then

        local releasingRodents = nest.energy < nest.prototype.energy

        if not releasingRodents then
            local player = pk2.getPlayerIfAccessible()

            if player~=nil and math.abs(player.x - nest.x) <= 20 and  math.abs(player.y - nest.y) <= 320 then
                nest.energy = nest.energy - 1
                nest.attack1Timer = 50
            end
        end

        if releasingRodents and nest.attack1Timer==0 then
            local rodent = pk2.addSprite(protoRodentClimbing, nest.x, nest.y, tree)
            rodent.target = tree
            rodent.b = -2

            nest.energy = nest.energy - 1
            if nest.energy<=1 then
                tree.target = nil
                nest:transform()
            else
                nest.attack1Timer = 200
            end
        end

    end

    pk2.forEachCreature(function (rodent)
        if rodent.prototype == protoRodentWalking and not rodent.canMoveDown and
        math.abs(rodent.x - tree.x) < 20

        then
            rodent:transform()
            rodent.target = tree
            rodent.x = tree.x
            rodent.y = rodent.y - 26
            rodent.b = -2
        end
    end)

    return false
end



local bunnyCounter = 0

function forestlib2.forestBunny(bunny)

    bunnyCounter = bunnyCounter + 1

    local proto = bunny.prototype
    if not bunny.canMoveDown and bunny.b <= 0 then

        if bunny.x < forestlib2.startX then
            bunny.flipX = false
        elseif bunny.x > forestlib2.stopX then
            bunny.flipX = true
        else

            local player = pk2.getPlayerIfAccessible()
            if player~=nil then

                if math.random(0, 4) == 1 then
                    if player.x < bunny.x then
                        bunny.flipX = true
                    else 
                        bunny.flipX = false
                    end
                end
            end

        end

    end

    if bunny.flipX then
        bunny.a = -proto.maxSpeed
    else
        bunny.a = proto.maxSpeed
    end

    return true
    
end


local spawnBunnyTimer = 200

function forestlib2.updateForest()
    local player = pk2.getPlayerIfAccessible()
    if player~=nil and player.x >= forestlib2.startX + 800 and player.x <= forestlib2.stopX - 800
    and player.y >= forestlib2.startY and player.y < forestlib2.stopY
    then
        if bunnyCounter < 2 then

            if spawnBunnyTimer <= 0 then
                spawnBunnyTimer= math.random(400, 800)

                local flipX

                --print("Zajonc")
                if player.a > 2. then
                    flipX = true
                elseif player.a < -2. then
                    flipX = false
                else
                    flipX = math.random(0, 1)==1
                end

                local bunnyX
                if flipX then
                    bunnyX = player.x + 432
                else
                    bunnyX = player.x - 432
                end

                local bunny = pk2.addSprite(protoForestBunny, bunnyX, forestlib2.stopY - 96, nil)
                bunny.flipX = flipX

            else
                spawnBunnyTimer = spawnBunnyTimer - 1
            end
            
        end       
    end

    bunnyCounter = 0
end


return forestlib2