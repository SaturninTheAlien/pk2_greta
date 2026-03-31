
local spiderlib = {}

local function isSolidForSpider(sector, x, y)

    local bgTile = sector:getBgTile(x, y)
    if bgTile~=91 and bgTile~=101 then
        return true
    end

    return sector:getFgTile(x, y) < 40
end


function spiderlib.bounce(spider)
    local sector = spider.levelSector

    local dx
    if spider.flipX then
        dx = -20
    else
        dx = 20
    end

    local tx, ty
    tx = math.floor((spider.x + dx)/32)
    ty = math.floor(spider.y / 32)

    if isSolidForSpider(sector, tx, ty) then
        spider.a = -spider.a
        spider.flipX = not spider.flipX
    end


    local dy
    if spider.b < 0 then
        dy = -20
    else
        dy = 20
    end

    tx = math.floor(spider.x /32)
    ty = math.floor((spider.y + dy) / 32)

    if isSolidForSpider(sector, tx, ty) then
        spider.b = -spider.b
    end

    return false
end


return spiderlib