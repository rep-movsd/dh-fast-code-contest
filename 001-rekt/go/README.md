# Runner for Go

## Instructions for writing Go implementation

Your `rekt.go` must contain these 2 functions:

1. `func Init(filename string)`
    - `filename` is the name of the file containing all the points

2. `func Run(rects []rect) [][]int32`
    - recieve a slice of rectangles and return a slice of a slice of ranks

## Instructions for running

### Compiling

Compile the go implementation:

    $ go build -buildmode=plugin -o librekt.so rekt.go

Compile the runner:

    $ go build -o runner runner.go

### Running

Run and check:

    $ ./runner <hash> <library>

Example:

    $ ./runner 59EEA7F5 ./librekt.so
