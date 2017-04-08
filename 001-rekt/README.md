
#### Intro


A simple problem statement and test data generator is provided and you have 4 weeks in which you can submit as many solutions as you like.

A monetary prize will be given to the winners (maximum two winners) - Amounts will be discussed on the slack channel


The submissions will be divided into 2 categories based on langauge demographics

 * Compiled languages - C/C++/Rust/Go/Java/C# etc. - no inline assembler or GPGPU (but compiler intrinsics allowed)
 * Dynamic languages - python/ruby/JS

If you are in category 1, you will have to create a shared object .so that can be called from a C program, so that we need not write a testing harness for every language.

Category 2 will need individual test scripts that we will write for each language.

We are keeping 2 classes because otherwise the compiled solutions will always dominate. 
There is no rule you have to use 1 language for all your submissions, feel free to switch

At the end the absolutely fastest code submission per person is only considered  and the top winner in each class wins half the prize money.

We are keeping 2 classes because otherwise the compiled solutions will always dominate. There is no rule that you have to use only one language for all your submissions.

If any dynamic language program manages to get top spot, then it wins the entire prize money, because the programmer has rekt all the Gurus.

The sole criteria for dynamic/static language is whether you can produce a self standing .so file - If it needs external dependencies, no problem, just include the build instructions

For now not supporting Windows - you can use it to develop and test, but final submission is a Linux .so
The reason is the system we use for testing all programs has to be a single one and adding Windows to the mix is needless complexity and may not reflect the same relative time.

If anyone has suggestions for this, let me know - perhaps AWS or some remote server can support Windows and Linux on the exact same CPU config?

---

#### Problem statement

You are given a file containing millions of rows each describing a point in 2D space:

    X Y rank
    X Y rank
    ...

X and Y are floating point values in the range
    
    -10000000000.0 <= val <= 10000000000.0   

rank is an integer, unique for each row, higher rank means lower value.... i.e. 0th rank is the highest possible 

You are then given a list of rectangles, and for each rectangle, you need to find the 20 highest ranked points in that rectangle, and return their indices in the input file.

A point px,py is inside a rectangle if 
  rc.lx <= px && px < rc.hx && rc.ly <= py && py < rc.hy


Your implementation needs to provide 2 APIs

 * init(filename) - reads the input file, sets up any data structures etc.
 * run(list of rects) - calculates the 20 highest ranked points for the given rectangles
 * results() - returns the calculated results

A generator program is included - you can use the static binary on Linux, or compile generate.cpp as follows
    
    g++ -O3 --std=c++11 -static -static-libgcc -static-libstdc++ -ogenerate generate.cpp

It generates (repeatably) random data sets, including the expected answers

For example running the following:

    $ ./generate 1000000 100 -
    Random seed : 1AAF9E9E
    Generating 100 rects
    Generating 1000000 points

creates

    1AAF9E9E-points.txt  
    1AAF9E9E-rects.txt
    1AAF9E9E-results.txt


The format of the points file is

    X Y rank
    X Y rank
    ...

X and Y are floating point values in the range
    
    -10000000000.0 <= val <= 10000000000.0   


The rect file has:
    
    LX HX LY HY
    LX HX LY HY
    ...

They represent the bounds of the rectangle 
LX <= HX and LY <= HY

The results file has one line for each rect and each line has the rank of upto 20 points separated by a space

   rank1 rank2 ...
   rank1 rank2 ...

If there are no points in any given rect, the line is blank


A runner program is provided that can load a .so that contains your implementation and time and verify your solution 
A sample .so written in C++ is provided 

You can build the runner and test implementation like this:

    g++ -O3 -rdynamic -o runner runner.cpp -ldl
    gcc -O3 -shared -o librekt.so -fPIC skeleton.cpp

Then you can run the test as follows:

    $ ./runner 1AAF9E9E ./librekt.so -
    Loading points from 1AAF9E9E-points.txt
    Checking results...OK
    209.534 ms elapsed
    2.09534 ms per rect

---

#### Prize money

The prize is divided between the top winners of two groups equally.

The winning entries have to have at least 5% better speed than the next best program. 
Otherwise we have a tie and the prize chunk is divided amongst the tied people.

If someones dynamic program takes top spot, entire prize money goes to them.


