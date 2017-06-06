package main

import (
	"bufio"
	"log"
	"os"
	"sort"
	"strconv"
	"strings"
)

// Init loads the points and stores them in some datastructure
func Init(filename string) {
	points = make([]point, 10000000)

	file, err := os.Open(filename)
	if err != nil {
		log.Fatal(err)
	}

	fscanner := bufio.NewScanner(file)
	for fscanner.Scan() {
		vals := strings.Split(fscanner.Text(), " ")
		x, _ := strconv.ParseFloat(vals[0], 32)
		y, _ := strconv.ParseFloat(vals[1], 32)
		rank, _ := strconv.ParseInt(vals[2], 10, 32)
		p := point{float32(x), float32(y), int32(rank)}
		points[rank] = p
	}

	makePairs()
	makeGoroutines(16)
}

func makePairs() {
	pointCount := len(points)
	pairCount, start, end := countPairs(pointCount)
	rankedPoints = points[:start]

	pairs := make([]pair, pairCount)
	recvCh := make(chan indexedPair)

	index := 0
	for start+end <= pointCount {
		if start+(end*4) > pointCount {
			end = pointCount - start
		}
		go makeAsyncPair(index, points[start:start+end], recvCh)
		start, end, index = start+end, end*3, index+1
	}

	for i := 0; i < pairCount; i++ {
		select {
		case idPair := <-recvCh:
			pairs[idPair.Index] = idPair.Pair
		}
	}

	partitions = pairs
}

func countPairs(num int) (int, int, int) {
	ranked := 2048
	start := ranked
	end := 3 * num / 10000
	count := 0
	for start+end <= num {
		if start+end*4 > num {
			end = num - start
		}
		count++
		start, end = start+end, end*3
	}
	return count, ranked, 3 * num / 10000
}

func makeAsyncPair(index int, pts []point, ch chan indexedPair) {
	p := makePair(pts)
	ch <- indexedPair{index, p}
}

func makePair(pts []point) pair {
	x := make([]point, len(pts))
	y := make([]point, len(pts))
	copy(x, pts)
	copy(y, pts)

	ch := make(chan indexedPartition)
	var p pair
	go makeAsyncPartition(1, x, ch)
	go makeAsyncPartition(2, y, ch)

	for i := 0; i < 2; i++ {
		select {
		case val := <-ch:
			if val.Index == 1 {
				p.XSorted = val.Partition
			} else {
				p.YSorted = val.Partition
			}
		}
	}

	return p
}

func makeAsyncPartition(index int, pts []point, ch chan indexedPartition) {
	switch index {
	case 1:
		sort.Slice(pts, func(i, j int) bool {
			return pts[i].X < pts[j].X
		})
	case 2:
		sort.Slice(pts, func(i, j int) bool {
			return pts[i].Y < pts[j].Y
		})
	}
	ch <- indexedPartition{index, makePartition(pts)}
}

func makePartition(pts []point) partition {
	var x, y []float32
	var rank []int32

	for _, p := range pts {
		x = append(x, p.X)
		y = append(y, p.Y)
		rank = append(rank, p.Rank)
	}

	return partition{x, y, rank}
}

func makeGoroutines(num int) {
	rectChan = make(chan indexedRect)
	for i := 0; i < num; i++ {
		go worker(rectChan)
	}
}

func main() {}
