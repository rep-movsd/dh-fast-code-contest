--[[
  dh-fastest-code-content
]]

local utils = require "utils.lua"
local collections = require "collections.lua"

local exports = {}

local internals = {
    points = nil,
    results = {},
}

function exports.init(filename)

    assert(filename ~= nil)

    local p = {}
    for line in io.lines(filename) do
        local pdata = utils.split(line)
        local point = {
            x = tonumber(pdata[1]),
            y = tonumber(pdata[2]),
            rank = tonumber(pdata[3]),
        }

        table.insert(p, point)
    end

    internals.points = collections.Kdtree:new(p)
end

function exports.run(rects)

    local internals = internals
    local table_insert = table.insert

    for i = 1, #rects do 
        local r = rects[i]
        local result = collections.MagicHeap:new(20)
        
        internals.points:find(r, function(point)
            result:insert(point.rank)
        end)

        result = result:sort()
        table_insert(internals.results, result)
    end
end

function exports.results()
    local result = ""
    for i = 1, #internals.results do
        local line = internals.results[i]
        for k, v in ipairs(line) do
            result = result  .. v .. " "
        end
        result = result .. "\n"
    end
    return result
end

return exports
