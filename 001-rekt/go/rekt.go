package main

import (
	"bufio"
	"log"
	"os"
	"sort"
	"strconv"
	"strings"
)

type point struct {
	X    float32
	Y    float32
	Rank int32
}

type rect struct {
	Lx float32
	Ly float32
	Hx float32
	Hy float32
}

var points []point

func Init(filename string) {
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
		points = append(points, p)
	}

	sort.Slice(points, func(i, j int) bool {
		return points[i].Rank < points[j].Rank
	})
}

func Run(rects []rect) (answer [][]int32) {
	for _, r := range rects {
		ranks := findRanks(r)
		answer = append(answer, ranks)
	}

	return
}

func findRanks(r rect) (ranks []int32) {
	for _, p := range points {
		if len(ranks) == 20 {
			return
		}
		if r.Lx <= p.X && p.X < r.Hx && r.Ly <= p.Y && p.Y < r.Hy {
			ranks = append(ranks, p.Rank)
		}
	}
	return
}
