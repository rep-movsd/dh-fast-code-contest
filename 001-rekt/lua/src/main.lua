--[[
--  dh-fastest-code-content
--]]

-- required to enable imports from the src directory
package.path = "./src/?.lua;"..package.path

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

function exports.results(buffer)
end

return exports
