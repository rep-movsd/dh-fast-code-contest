
#### Intro


A simple problem statement and test data provided and you have 4 weeks in which you can submit as many solutions as you like.

A monetary prize will be given to the winners (maximum two winners) - Amounts will be discussed on the slack channel


The submissions will be divided into 2 categories based on langauge demographics

 * Compiled languages - C/C++/Rust/Go/Java/C# etc. - no inline assembler or GPGPU (but compiler intrinsics allowed)
 * Dynamic languages - python/ruby/JS

Multiprocessing of any kind is not allowed - Solutions will be tested on a single core machine

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

---

#### Problem statement

Given an RGB image with 24 bit colors (16777216 distinct colors) one can define a palette of 256 out of the possible 16 million
Using colors in this palette alone we can recreate the image such that it looks like a reasonable facsimile of the original  

This is the basis for 8 bit color images like 8-bit PNG and GIF formats

When converting a 24 bit color image (also called TrueColor) to an 8 bit color depth, two operations are performed

  * First we need to get the best 256 colors
  * Then we need to scan through the image and for each 24 bit pixel, we decide which of the 256 palette colors we need to use for that
  
The first part is a very complex and there are many algorithms that are commonly used like Median Cut and Octrees - we don not concern ourselves with that for this challenge
The second part is simple to state but hard to do efficiently without using a lookup table.

You are given a file containing 256 rows each describing a RGB color (we will refer to this as the palette file):

    R G B 
    R G B 
    ...

R, G and B and A are integer values in the range
    
    0 <= val <= 255   


You are also given a file containing several million 24 bit values each representing a pixel in an image

    RGB RGB RGB ...

Each RGB are integer values in the range
    
    0 <= val <= 0xFFFFFF   


Remember that we are in little-endian mode so if you have a 32 bit integer value in HEX 0x00345678 then 78 is R, 56 is G, 34 is B
Static-typed language programmers should not have to worry about this, but dynamic people make sure you get it right

The challenge is to produce an output file containing one entry for each pixel, ranging from 0 to 255
The output values are the index of the palette color which match the input pixel color closest.

We define closeness of two colors as the cartesian distance between them in RGB 3D space.
If we have two colors R1, G1, B1 and R2, G2, B2, distance between them is given by:

    SQRT(ABS(R1-R2)^2 + ABS(G1-G2)^2 + ABS(B1-B2)^2)
    
This is nothing but the basic distance formula, visualized with the 3 axes being red, green and blue
It is possible (but very unlikely) that two palette colors match an input color, in that case choose arbitrarily

The "average error" of the file is calculated as the RGB distance between each input and output pixel averaged over the whole image and scaled to a percentage

The winner is the program that runs the fastest and gives an error < 5% (this value may be revised later after experiments)

Your implementation needs to provide 2 APIs

 * init(filename) - reads the palette file, sets up any data structures etc.
 * run(filename) - reads the pixel file and writes an output file


Scripts will be provided to generate test data from real images 
 
---

#### Prize money

The prize is divided between the top winners of two groups equally.

The winning entries have to have at least 5% better speed than the next best program. 
Otherwise we have a tie and the prize chunk is divided amongst the tied people.

If someones dynamic program takes top spot, entire prize money goes to them.


