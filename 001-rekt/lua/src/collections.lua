local exports = {}

local MagicHeap = {}
MagicHeap.__index = MagicHeap
exports.MagicHeap = MagicHeap

function default_cmp(x, y)
    return x > y
end

function MagicHeap:new(cap, cmp)

    local obj = { 
        cap = cap,
        items = {}, 
        len = 0,
        cmp = cmp or default_cmp
    }
    return setmetatable(obj, self)
end

function MagicHeap:_sift()

    local items = self.items
    local cmp = self.cmp
    local len = self.len

    local p = 1
    local c1 = p * 2
    local c2 = c1 + 1

    while c1 <= len do

        local n

        if c2 > len then
            n = c1
        elseif cmp(items[c1], items[c2]) then
            n = c1
        else
            n = c2
        end

        if cmp(items[p], items[n]) then
            break
        end

        items[p], items[n] = items[n], items[p]
        p = n
        c1 = p * 2
        c2 = c1 + 1
    end
end

function MagicHeap:insert(item)

    local items = self.items
    self.len = self.len + 1
    items[self.len] = item

    local i = self.len
    local p = math.floor(i/2)
    local cmp = self.cmp
    
    -- let it sift up
    while i > 1 and not cmp(items[p], items[i]) do
        items[i], items[p] = items[p], items[i]
        i = p
        p = math.floor(i/2)
    end

    if self.len > self.cap then
        items[1] = items[self.len]
        items[self.len] = nil
        self.len = self.len -1
        self:_sift()
    end
end

function MagicHeap:sort()

    local items = self.items

    while self.len > 1 do
        items[self.len], items[1] = items[1], items[self.len]
        self.len = self.len -1
        self:_sift()
    end

    return items
end

return exports
