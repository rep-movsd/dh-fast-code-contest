--[[
  dh-fastest-code-content
]]

local utils = require "utils.lua"

local exports = {}
local internals = {
    points = {},
    results = {}
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

    local function compare(p1, p2)
        return p1.rank < p2.rank
    end

    table.sort(internals.points, compare)
end

function exports.run(rects)
    for i = 1, #rects do 
        local r = rects[i]
        local result = {}
        for j = 1, #internals.points do 
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
