package main

import (
	"container/heap"
	"sort"
	"time"
)

type indexedTime struct {
	Index    int
	Duration time.Duration
}

// Run the program against the given reactangle
func Run(rects []rect) [][]int32 {
	result = make([][]int32, len(rects))

	for i, r := range rects {
		hr := &rankHeap{}
		heap.Init(hr)
		job := indexedRect{i, r, hr}
		rectChan <- job
	}

	close(rectChan)
	return result
}

func worker(queue <-chan indexedRect) {
	for job := range queue {
		rect := job.Rect
		ranks := runRect(rect.Lx, rect.Ly, rect.Hx, rect.Hy, job.Heap)
		result[job.Index] = ranks
	}
}

func runRect(x1, y1, x2, y2 float32, hr *rankHeap) (ranks []int32) {
	for _, p := range rankedPoints {
		if len(ranks) == 20 {
			return ranks
		}
		if x1 <= p.X && p.X < x2 && y1 <= p.Y && p.Y < y2 {
			ranks = append(ranks, p.Rank)
		}
	}

	for _, p := range partitions {
		xStart, xEnd := findRangeX(p.XSorted, x1, x2)
		yStart, yEnd := findRangeY(p.YSorted, y1, y2)

		if xEnd-xStart > yEnd-yStart {
			findRanksY(p.YSorted, yStart, yEnd, x1, x2, hr)
		} else {
			findRanksX(p.XSorted, xStart, xEnd, y1, y2, hr)
		}

		for hr.Len() > 0 && len(ranks) < 20 {
			ranks = append(ranks, heap.Pop(hr).(int32))
		}

		if len(ranks) == 20 {
			return ranks
		}
	}

	return
}

func findRanksX(p partition, start, end int, y1, y2 float32, hr *rankHeap) {
	findRanks(p.Y, p.Rank, start, end, y1, y2, hr)
}

func findRanksY(p partition, start, end int, x1, x2 float32, hr *rankHeap) {
	findRanks(p.X, p.Rank, start, end, x1, x2, hr)
}

func findRanks(list []float32, rankList []int32, start, end int, p1, p2 float32, hr *rankHeap) {
	for i := start; i < end; i++ {
		if p1 <= list[i] && list[i] < p2 {
			heap.Push(hr, rankList[i])
		}
	}
	return
}

func findRangeX(p partition, x1, x2 float32) (int, int) {
	return findRange(p.X, x1, x2)
}

func findRangeY(p partition, y1, y2 float32) (int, int) {
	return findRange(p.Y, y1, y2)
}

func findRange(list []float32, p1, p2 float32) (int, int) {
	start := sort.Search(len(list), func(i int) bool {
		return list[i] >= p1
	})

	end := sort.Search(len(list), func(i int) bool {
		return list[i] >= p2
	})

	return start, end
}
