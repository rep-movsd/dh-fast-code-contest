--[[
--  dh-fastest-code-content
--]]

local utils = {}

--[[
--  not the best algorithm to split a string
--]]
utils.split = function(str, delim)

    assert(str ~= nil)

    delim = delim or " "

    local chunks = {}
    local last_idx = 1
    for idx = 1, #str do
        local chr = str:sub(idx, idx)
        if chr == delim then
            local x = str:sub(last_idx+1, idx-1)
            last_idx = idx
            table.insert(chunks, x)
        end
    end

    if last_idx ~= #str then
        table.insert(chunks, str:sub(last_idx))
    end

    return chunks
end

utils.tostring = function(entity)

    if type(entity) ~= "table" then
        -- this is the builtin tostring
        return tostring(entity)
    end

    local str = "{ "
    for key, value in pairs(entity) do
        str = str .. string.format("%s = %s", key, value) .. ","
    end
    str = str:sub(1, #str-1) .. " }"
    return str
end

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
