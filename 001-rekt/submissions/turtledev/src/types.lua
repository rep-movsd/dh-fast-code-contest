--[[
    type definitions
]]

local ffi = require "ffi"

ffi.cdef [[
typedef struct {
    double x;
    double y;
    int64_t rank;
} Point;

typedef struct {
    double lx, ly;
    double hx, hy;
} BoundingBox;

typedef struct _node {
    Point *point;
    struct _node *left;
    struct _node *right;
    BoundingBox bounding_box;
} Node;

void qsort(void *base, size_t nmemb, size_t size, int (*cmp)(const void *, const void*));

void *malloc(size_t);
]]
