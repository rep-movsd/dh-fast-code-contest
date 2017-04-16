--[[
  dh-fastest-code-content
]]

local utils = require "utils.lua"
local collections = require "collections.lua"

--[[
  binary search-a-like

  returns the closest index
]]
function approx(vec, value, map)
    local start = 1
    local len = #vec
    local fin = len
    map = map or function(x) return x end
    while true do
        local idx = math.ceil((fin + start)/2)
        idx = utils.clamp(idx, 1, len)
        local current = map(vec[idx]) 

        if current == value or start > fin then
            return idx
        elseif value > current then
            start = idx + 1
        elseif value < current then
            fin = idx - 1
        end 
    end
end

function map(vec, f)
    local result = {}
    for i = 1, #vec do
        result[i] = f(vec[i])
    end
    return result
end

function stddev(values)
    -- compute the mean
    local mean = 0
    for i = 1, #values do
        mean = mean + values[i]
    end
    mean = mean / #values

    -- compute deviations
    local deviations = map(values, function(v) return (v - mean)^ 2 end)
    local variance = 0
    for i = 1, #deviations do
        variance = variance + deviations[i]
    end
    variance = variance / #deviations
    
    -- standard deviation is the square root of variation
    return math.sqrt(variance)
end

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
