local pk2 = require("pk2")

local protoTree = pk2.loadSpritePrototype("rtree.spr2")
local protoBranch = pk2.loadSpritePrototype("rtree_branch.spr2")
local protoBranch2 = pk2.loadSpritePrototype("rtree_branch2.spr2")
local protoTreePlatform = pk2.loadSpritePrototype("rtree_platform.spr2")
local protoNest = pk2.loadSpritePrototype("rtree_nest1.spr2");

local protoApple = pk2.loadSpritePrototype("apple.spr2")


local protoFungus1 = pk2.loadSpritePrototype("fungus1.spr2")
local protoBgTree = pk2.loadSpritePrototype("bg_tree.spr2")
local protoBgTree2 = pk2.loadSpritePrototype("bg_tree2.spr2")


local forestLib = {}


local function addBranch(sector, tree, flipX, dy)
    local posX
    if flipX then
        posX = tree.x + 44
    else
        posX = tree.x - 49
    end

    local posY = tree.y + 80 + dy
    local branch = sector:addSprite(protoBranch, posX, posY, tree);
    branch.flipX = flipX

    sector:addSprite(protoTreePlatform, posX, posY - 13, branch)

    return branch
end

local function addBranch2(sector, posX, posY, tree)

    local branch = sector:addSprite(protoBranch2, posX, posY, tree)

    if math.random(0, 1)== 1 then
        branch.flipX = true
    end

    sector:addSprite(protoApple, posX, posY - 30, branch);
    sector:addSprite(protoTreePlatform, posX, posY, branch)

    return branch
end

local function addRodentNest(sector, tree)
    local nest = sector:addSprite(protoNest, tree.x + math.random(-4, -2), tree.y + 100 + math.random(0, 40), tree)
    nest.flipX = math.random(0, 1)==1
    tree.target = nest
    return nest
end


function forestLib.addTree(sector, posX, posY, hasRodents)
    local tree = sector:addSprite(protoTree, posX, posY + 32 - protoTree.height/2, nil)

    if hasRodents then
        addRodentNest(sector, tree)
    end


    local branchLeft = addBranch(sector, tree, false,  math.random(0,40))
    local branchRight = addBranch(sector, tree, true, math.random(0,40))

    local branchesNumber = math.random(0, 5)

    local minY = tree.y - 120
    local maxY = tree.y + 35
    local maxX, minX, x, y

    local y1 = math.min(branchLeft.y, branchRight.y) - 70

    local minX2 = tree.x + protoBranch2.width // 2
    local maxX2 = tree.x - protoBranch2.width // 2

   
    for i=1,branchesNumber do
        y = y1 + math.random(-10, 10)

        if y > maxY then
            y = maxY

        elseif y < minY then
            y = minY
        end


        if y <= tree.y - 100 then

            minX = tree.x - 20
            maxX = tree.x + 20

        elseif y <= tree.y - 60 then

            minX = tree.x - 40
            maxX = tree.x + 40

        else
            minX = tree.x - 55
            maxX = tree.x + 55
        end

        if i%2 == 0 then
            y1 = y1 - 70
            minX = minX2
        elseif i~=branchesNumber then
            maxX = maxX2

            if branchesNumber < 4 and math.random(0, 1)==0 then
                y1 = y1 - 70
            end
        end


        x = math.random(minX, maxX)

        addBranch2(sector, x, y, tree)
    end

    return tree, math.min( branchLeft.y - tree.y, branchRight.y - tree.y )
end


function forestLib.addFungus(sector, x, y)
    local fungus = sector:addSprite(protoFungus1, x, y, nil)
    sector:addSprite(protoTreePlatform, x, y - 27, fungus)

    fungus.flipX = math.random(0, 1)==1


    return fungus
end

function forestLib.genForest(sector, startTileX, stopTileX, tileY)
    for tileX=startTileX,stopTileX do
        local tileType = math.random(-1, 20)
        local fgTile = 255

        if tileType == -1 then
            fgTile = 125
        elseif tileType == 0 then
            fgTile = 88
        elseif tileType == 1 then
            fgTile = 87
        elseif tileType <= 4 then
            fgTile = 96
        elseif tileType <= 8 then
            fgTile = 98
        elseif tileType <= 12 then
            fgTile = 85
        elseif tileType <= 18 then
            fgTile = 86
        end

        sector:setFgTile(tileX, tileY, fgTile)

    end


    local startX = 32* startTileX;
    local stopX = 32 * stopTileX;
    local y = 32 * tileY;


    local x = startX + math.random(160, 640)
    while x < stopX do

        local bgTreeType;

        if math.random(0, 1)==1 then
            bgTreeType = protoBgTree;
        else
            bgTreeType = protoBgTree2;
        end

        local bgTree = sector:addSprite(bgTreeType, x, y)
        bgTree.flipX = math.random(0, 1)==1

        x = x + math.random(320, 640)
    end

    x = startX

    local distanceToPrev = 480

    while x < stopX do
        local distanceToNext = math.random(320, 640)

        local _, var1 = forestLib.addTree(sector, x, y+16, math.random(0, 4)==0)

        if var1 < 100 then
            local fungus_x
            if distanceToNext > distanceToPrev then
                fungus_x = x + math.random(120, 200)
            else
                fungus_x = x - math.random(120, 200)
            end

            forestLib.addFungus(sector, fungus_x, y)
        end

        x = x + distanceToNext
        distanceToPrev = distanceToNext
    end

end



return forestLib