
####Intro

A simple problem statement and test data sets will be provided and you have 4 weeks in which you can submit as many solutions as you like.

The submissions will be divided into 2 categories based on langauge demographics

1) Compiled languages - C/C++/Rust/Go/Java/C# etc. - no inline assembler or GPGPU (but compiler intrinsics allowed)
2) Dynamic languages - python/ruby/JS

If you are in category 1, you will have to create a shared object .so or .dll that can be called from a C program, so that we need not write a testing harness for every language.

Category 2 will need individual test scripts that we will write for each language.

We are keeping 2 classes because otherwise the compiled solutions will always dominate. There is no rule you have to use only 1 language. 

At the end the absolutely fastest code submission per person is only considered  and the top winner in each class wins half the prize money.

We are keeping 2 classes because otherwise the compiled solutions will always dominate. There is no rule that you have to use only one language for all your submissions.

If any dynamic language manages to get top spot, then it wins full prize money.

The sole criteria for dynamic/static language is whether you can produce a self standing .so file with no dependencies.

For now not supporting Windows - you can use it to develop and test, but final submission is a Linux .so
The reason is the system we use for testing all programs has to be a single one and adding Windows to the mix is needless complexity and may not reflect the same relative time.

If anyone has suggestions for this, let me know - perhaps AWS or some remote server can support Windows and Linux on the exact same CPU config?


#### Problem statement

You are given a file containing 1000000 rows each describing a point in 2D space:

X Y rank
X Y rank
...

X and Y are floating point values in the range
-10000000000.0 <= val <= 10000000000.0   

rank is an integer, unique for each row, higher rank means lower value.... i.e. 0th rank is the highest possible 

You are then given a list of rectangles, and for each rectangle, you need to find the 20 highest ranked points in that rectangle, and return their indices in the input file.

Your implementation needs to provide 2 APIs

1) init(filename) - reads the input file, sets up any data structures etc.
2) run(list of rects) - calculates the 20 highest ranked points for the given rectangles

We will provide a reference implementation that lets you verify your results during development.
 

Prize money

The prize is divided between the top winners of two groups, 5050.50 each.

The winning entries have to have at least 5% better speed than the next best program. Otherwise we have a tie and the 5050.50 is divided amongst the tied people.

If someones dynamic program takes top spot, entire prize money of 10101.01 goes to them.


####Misc

Since we need to setup testing etc. with many languages we wont officially kick off the contest until there is at least one basic solution running per language that is being used.

Those who make .so etc. share the skeleton code for others, so that people can get started easily on the main job and not struggle with the meta-tasks.

We will provide random data sets for you to test locally and a tool to help generate more 

If you are dead set on using a particular pet language, go ahead with your local testing, we will manage to write the tester by the time contest ends (but please, no Brainfuck or other mental languages)

If you use Haskell (such #pru, much functional, wow) , please write the tester yourself based on the ones we write and pass it on.

After the contest officially starts, you have upto 30 days to submit as many times as you like. We will try to update a leaderboard everytime a new submission is made and at least once a day.


This document will be updated as time passes



