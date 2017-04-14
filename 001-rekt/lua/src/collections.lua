--[[
  basic heap implementation
]]

local utils = require "utils.lua"

local exports = {}

local Heap = {}
Heap.__index = Heap
exports.Heap = Heap

function default_cmp(parent, child)
    return parent < child
end

function Heap:new(cmp)
    local obj = {
        length = 0,
        items = {},
        cmp = cmp or default_cmp
    }
    setmetatable(obj, self)
    return obj
end

function Heap:insert(val)

    assert(val, "missing argument to Heap#insert()")

    -- insert the new value at the end
    self.length = self.length + 1
    self.items[self.length] = val

    local items = self.items
    local cmp = self.cmp
    local c = self.length
    local p = math.floor(c/2)

    -- let it move up it's place
    while c ~= 1 and not cmp(items[p], items[c]) do
        items[p], items[c] = items[c], items[p]
        c = p
        p = math.floor(c/2)
    end
end

-- sift the top most element down to it's position in a heap
function sift(items, length, cmp)

    local p = 1
    local c1 = p * 2
    local c2 = c1 + 1

    while c1 <= length do

        local n

        -- select one of the two childs
        if c2 > length then
            n = c1
        elseif cmp(items[c1], items[c2]) then
            n = c1
        else
            n = c2
        end

        -- the node is in the right place
        if cmp(items[p], items[n]) then
            break
        end

        -- sift down
        items[p], items[n] = items[n], items[p]
        p = n
        c1 = p * 2
        c2 = c1 + 1
    end
end

function Heap:sort()

    local items = utils.slice(self.items)
    local length = self.length
    local cmp = self.cmp

    while length > 1 do
        items[length], items[1] = items[1], items[length]
        length = length -1
        sift(items, length, cmp)
    end

    return items
end
    
return exports
