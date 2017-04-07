--[[
--  dh-fastest-code-content
--]]

local utils = require("utils")

local exports = {}
local internals = {
    points = {}
}

function exports.init(filename)

    assert(filename ~= nil)

    for line in io.lines(filename) do
        local pdata = utils.split(line)
        local point = {
            x = pdata[1],
            y = pdata[2],
            rank = pdata[3]
        }
        table.insert(internals.points, point)
    end

    local function compare(p1, p2)
        return p1.rank < p2.rank
    end

    table.sort(internals.points, compare)
end

function exports.run(rects)
end

function exports.results()
end

return exports
