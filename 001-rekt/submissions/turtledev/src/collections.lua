--[[
    collections -- data structures
]]

local ffi = require "ffi"
local utils = require "utils.lua"

local exports = {}

local MagicHeap = {}
MagicHeap.__index = MagicHeap
exports.MagicHeap = MagicHeap

--[[
    MagicHeap -- A presized heap
    
    MagicHeap has a limit on the number of elements that can be inserted
    into it. When the heap is full and a new element is inserted, it inserts
    the new element into place, and one of the other nodes is removed. 

    Internally, it always keeps an extra slot to make this insertion/removal
    possible.

    Methods:

    MagicHeap:new(capacity, comparator)
        constructor -- creates a new instance of MagicHeap

        @param capacity {number} the heap capacity, 
        @param comparator {function(a, b)} The comparator function
        @returns {MagicHeap}

    MagicHeap:top()
        returns the the root of the heap

        @returns {any}

    MagicHeap:insert(element)
        insert an element into the heap

        @param element {any}

    MagicHeap:sort()
        Sort the heap using heapsort algorithm and return a table containing
        the sorted elements.

        note: the current implementation is destructive in the sense that the
        object because unusable after MagicHeap:sort() method is called.
]]


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

function MagicHeap:bounds()

    local top, bottom
    if #self.items < self.cap then
        return nil
    end
    top = self.items[1]
    local i = math.floor(self.cap/2)
    bottom = self.items[i]
    while i <= self.cap do
        if self.cmp(bottom, self.items[i]) then
            bottom = self.items[i]
        end
        i = i + 1
    end

    return top, bottom
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


local axis_map = {}
axis_map[0] = 'y'
axis_map[1] = 'rank'
axis_map[2] = 'x'

local axis_cmp = {}
axis_cmp[0] = function(a, b) return a.y < b.y and -1 or 1 end
axis_cmp[1] = function(a, b) return a.rank - b.rank end
axis_cmp[2] = function(a, b) return a.x < b.x and -1 or 1 end

for i = 0, 2 do
    axis_cmp[i] = ffi.cast('int (*)(Point *, Point *)', axis_cmp[i])
end

function find(tree, rect, heap, depth)
    if tree == nil then
        return
    end

    depth = depth or 0
    local axis = axis_map[depth % 3]

    local p = tree[0].point
    local r = rect
    local box = tree.bounding_box;

    --[[
         if the bounding box of this node does not collide with query,
         then skip this subtree
    ]]
    if box.lx > r.hx or box.hx < r.lx or box.ly > r.hy or box.hy < r.ly then
        return
    end

    if r.lx <= p.x and p.x < r.hx and r.ly <= p.y and p.y < r.hy then
        heap:insert(p.rank)
    end

    if axis ~= 'rank' then
        local max = 'h'..axis
        local min = 'l'..axis
       
        if rect[min] <= p[axis] then
            find(tree[0].left, rect, heap, depth + 1)
        end

        if rect[max] > p[axis] then
            find(tree[0].right, rect, heap, depth + 1)
        end
    else
        local top, bottom = heap:bounds()
        if (not bottom) or p.rank < bottom then
            find(tree[0].left, rect, heap, depth + 1)
        end
        if (not top) or p.rank <= top then
            find(tree[0].right, rect, heap, depth + 1)
        end
    end
end

local Kdtree = {}
Kdtree.__index = Kdtree
exports.Kdtree = Kdtree

function load_tree(points, start, fin, depth)

    -- we slice into the points table to build our tree. 

    depth = depth or 0

    local len = fin - start

    if len <= 0 then
        return nil
    end

    local median = start + math.floor(len/2)

    local axis = axis_map[depth % 3]
    local cmp = axis_cmp[depth % 3]

    local base = points[start]
    ffi.C.qsort(base, len, ffi.sizeof('Point'), cmp)

    local lx, ly = points[median].x, points[median].y
    local hx, hy = lx, ly

    for i = start, fin -1 do
        if points[i].x < lx then lx = points[i].x end
        if points[i].y < ly then ly = points[i].y end
        if points[i].x > hx then hx = points[i].x end
        if points[i].y > hy then hy = points[i].y end
    end

    local node = ffi.C.malloc(ffi.sizeof('Node'))

    node = ffi.cast('Node *', node)
    node.bounding_box.lx = lx;
    node.bounding_box.ly = ly;
    node.bounding_box.hx = hx;
    node.bounding_box.hy = hy;

    node[0].point = points[median]
    node[0].left = load_tree(points, start, median, depth + 1)
    node[0].right = load_tree(points, median+1, fin, depth + 1)

    return node
end

--[[ 
    A kdtree implementation that can be bulkloaded.

    The tree is specific to searching Point(x, y, rank) nodes, i.e. it's
    not a generic kdtree.
    
    Methods:

    Kdtree:new(points)
        constructor -- bulk load a kdtree    

        @param points {ctype(Points *)} points to load
        @param len {number} length of the array


    Kdtree:find(rect)
        Find 20 top ranked nodes in the current tree, using heap
        to store intermediate result(s).

        @param rect {Rect} A table with lx, hx, ly and hy values

        The heap is also used to optimize search, so when possible, 
        load it up with as many top ranked points as you can.
]]

function Kdtree:new(points, len)
    local obj = {
        root = load_tree(points, 0, len)
    }
    setmetatable(obj, self)
    return obj
end

function Kdtree:find(rect)
    local heap = MagicHeap:new(20)
    find(self.root, rect, heap)
    return heap:sort()
end

return exports
