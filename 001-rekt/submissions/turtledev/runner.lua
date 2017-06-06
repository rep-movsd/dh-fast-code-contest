local program = nil

local split = function(str, delim)

    assert(str ~= nil)

    delim = delim or " "

    local chunks = {}
    local last_idx = 0
    local len = #str
    for idx = 1, len do
        local chr = str:sub(idx, idx)
        if chr == delim then
            local x = str:sub(last_idx+1, idx-1)
            last_idx = idx
            table.insert(chunks, x)
        end
    end

    if last_idx ~= len then
        table.insert(chunks, str:sub(last_idx))
    end

    return chunks
end

function strip(s) 
    local start = 1
    local fin = #s

    while string.match(s:sub(start, start), "%s") do
        start = start +1
        break
    end

    while string.match(s:sub(fin, fin), "%s") do
        fin = fin -1
        break
    end

    return string.sub(start, fin)
end

function main(argv)

    local seed = argv[1]
    local program_file = argv[2]
    assert(seed and program_file, string.format('usage: %s SEED PROGRAM', argv[0]))
    program = assert(loadfile(program_file))()
    local points_file = seed..'-points.txt'
    local rects_file = seed..'-rects.txt'
    local results_file = seed..'-results.txt'

    -- init the module
    program.init(points_file)

    -- load the rects
    local rects = {}
    for line in io.lines(rects_file) do
        local rval = split(line)
        local rect = {
            lx = tonumber(rval[1]),
            hx = tonumber(rval[2]),
            ly = tonumber(rval[3]),
            hy = tonumber(rval[4])
        }
        table.insert(rects, rect)
    end

    -- start bench marking
    local start_time = os.clock()
    program.run(rects)
    local end_time = os.clock()
    local runtime = (end_time - start_time) * 1000

    -- verify the result
    io.write('verifying result ...')
    local valid = true
    local result = split(program.results(), '\n')
    local k = 1
    for line in io.lines(results_file) do
        if strip(line) ~= strip(result[k]) then
            valid = false
            break
        end
        k = k + 1
    end

    if valid then
        print('OK')
    else
        print('NOT OK')
    end

    print(string.format('total time: %d ms', runtime))
    print(string.format('per rect: %f ms', runtime/#rects))

    return 0
end

os.exit(main(arg))
