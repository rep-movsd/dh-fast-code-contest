package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"plugin"
	"strconv"
	"strings"
	"time"
)

type rect struct {
	Lx float32
	Ly float32
	Hx float32
	Hy float32
}

func main() {
	if len(os.Args) != 3 {
		fmt.Println("Usage:", os.Args[0], "<hash> <lib>")
		return
	}

	hash := os.Args[1]
	pointsFile := hash + "-points.txt"
	rectsFile := hash + "-rects.txt"
	resultsFile := hash + "-results.txt"

	p, err := plugin.Open(os.Args[2])
	if err != nil {
		log.Fatal(err)
	}

	init, err := p.Lookup("Init")
	if err != nil {
		log.Fatal(err)
	}

	run, err := p.Lookup("Run")
	if err != nil {
		log.Fatal(err)
	}

	rects := loadRects(rectsFile)

	initStart := time.Now()
	init.(func(string))(pointsFile)
	fmt.Println("Time to load:", time.Since(initStart))

	runStart := time.Now()
	answers := run.(func([]rect) [][]int32)(rects)
	finalTime := time.Since(runStart)
	fmt.Println("Time to process:", finalTime)
	avgTime := finalTime.Nanoseconds() / int64(len(rects))
	fmt.Println("Average time:", time.Duration(avgTime))

	success := loadAndCompare(resultsFile, answers)
	if success {
		fmt.Println("Passes!")
	} else {
		fmt.Println("Failed!")
	}
}

func loadRects(filename string) (rects []rect) {
	file, err := os.Open(filename)
	if err != nil {
		log.Fatal(err)
	}

	fscanner := bufio.NewScanner(file)
	for fscanner.Scan() {
		r := strings.Split(fscanner.Text(), " ")
		lx, _ := strconv.ParseFloat(r[0], 32)
		hx, _ := strconv.ParseFloat(r[1], 32)
		ly, _ := strconv.ParseFloat(r[2], 32)
		hy, _ := strconv.ParseFloat(r[3], 32)

		newRect := rect{float32(lx), float32(ly), float32(hx), float32(hy)}
		rects = append(rects, newRect)
	}

	return
}

func loadAndCompare(filename string, answers [][]int32) bool {
	file, err := os.Open(filename)
	if err != nil {
		log.Fatal(err)
	}
	results := [][]int32{}
	fscanner := bufio.NewScanner(file)
	for fscanner.Scan() {
		ranks := parsePoints(fscanner.Text())
		results = append(results, ranks)
	}

	for i := 0; i < len(results); i++ {
		if len(results[i]) != len(answers[i]) {
			fmt.Println(answers[i])
			fmt.Println(results[i])
			return false
		}
		for j := 0; j < len(results[i]); j++ {
			if answers[i][j] != results[i][j] {
				fmt.Println(answers[i], results[i])
				return false
			}
		}
	}
	return true
}

func parsePoints(ranksStr string) []int32 {
	r := strings.Split(strings.TrimSpace(ranksStr), " ")
	rs := []int32{}
	for _, rank := range r {
		newRank, err := strconv.ParseInt(rank, 10, 32)
		if err != nil {
			continue
		}
		rs = append(rs, int32(newRank))
	}

	return rs
}
