package main

type point struct {
	X    float32
	Y    float32
	Rank int32
}

type partition struct {
	X    []float32
	Y    []float32
	Rank []int32
}

type pair struct {
	XSorted partition
	YSorted partition
}

type indexedPair struct {
	Index int
	Pair  pair
}

type indexedPartition struct {
	Index     int
	Partition partition
}

// Rect implements the rectangle type
type rect struct {
	Lx float32
	Ly float32
	Hx float32
	Hy float32
}

type indexedRect struct {
	Index int
	Rect  rect
	Heap  *rankHeap
}

var points []point
var rankedPoints []point
var partitions []pair

var rectChan chan indexedRect
var result [][]int32

type rankHeap []int32

func (h rankHeap) Len() int {
	return len(h)
}

func (h rankHeap) Less(i, j int) bool {
	return h[i] < h[j]
}

func (h rankHeap) Swap(i, j int) {
	h[i], h[j] = h[j], h[i]
}

func (h *rankHeap) Push(x interface{}) {
	*h = append(*h, x.(int32))
}

func (h *rankHeap) Pop() interface{} {
	old := *h
	n := len(old)
	x := old[n-1]
	*h = old[0 : n-1]
	return x
}
