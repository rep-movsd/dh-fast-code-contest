--[[
  dh-fastest-code-content
]]

local utils = require "utils.lua"
local collections = require "collections.lua"

utils.clamp = function(value, min, max)
    if value < min then
        return min
    elseif value > max then
        return max
    else
        return value
    end
end

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
    points = {},
    results = {},
    axis = nil
}

function exports.init(filename)

    assert(filename ~= nil)

    for line in io.lines(filename) do
        local pdata = utils.split(line)
        local point = {
            x = tonumber(pdata[1]),
            y = tonumber(pdata[2]),
            rank = tonumber(pdata[3]),
        }

        table.insert(internals.points, point)
    end

    -- determine what axis to select
    local xvalues = map(internals.points, function(p) return p.x end)
    local yvalues = map(internals.points, function(p) return p.y end)

    local xstddev = stddev(xvalues)
    local ystddev = stddev(yvalues)

    if ( xstddev < ystddev ) then
        internals.axis = 'x'
    else
        internals.axis = 'y'
    end

    local function compare(p1, p2)
        return p1[internals.axis] < p2[internals.axis]
    end

    table.sort(internals.points, compare)
end

function exports.run(rects)
    for i = 1, #rects do 
        local r = rects[i]
        local result = {}
        local start = approx(internals.points, r.lx, function(p) return p[internals.axis] end)
        local finish = approx(internals.points, r.hx, function(p) return p[internals.axis] end)
        for j = start, finish do 
            local p = internals.points[j]
            if r.lx <= p.x and p.x <= r.hx and r.ly <= p.y and p.y <= r.hy then
                table.insert(result, p.rank)
            end
        end
        table.sort(result, function(x, y) return x < y end)
        result = utils.slice(result, 1, 20)
        table.insert(internals.results, result)
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
