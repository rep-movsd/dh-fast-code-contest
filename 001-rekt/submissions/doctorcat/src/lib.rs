#![feature(rand, sort_unstable, test)]
#[macro_use] extern crate lazy_static;
extern crate order_stat;
#[cfg(test)] #[macro_use]
extern crate quickcheck;
extern crate rand;
extern crate rayon;
extern crate spade;
extern crate test;
extern crate itertools;

mod rpoint;
mod rtree;
mod solution;
#[allow(dead_code)]
mod tree;

use rtree::BoundingRect;
use rtree::RTree;
use rayon::Configuration;
use rpoint::RPoint;
use std::ffi::{CStr, CString};
use std::fmt::Write;
use std::fs::File;
use std::io::BufReader;
use std::io::prelude::*;
use std::os::raw::c_char;
use std::ptr;
use std::str::FromStr;
use std::sync::Mutex;
use solution::{process, find_solutions};


lazy_static! {
    static ref RTREES: Mutex<Vec<RTree<RPoint>>> = Mutex::new(Vec::new());
    static ref POINTS: Mutex<Vec<RPoint>> = Mutex::new(Vec::with_capacity(10000000));
    static ref RESULTS: Mutex<Vec<(usize, Vec<u32>)>> = Mutex::new(Vec::new());
}

const NUM_POINTS: usize = 20;

#[repr(C)]
#[repr(packed)]
pub struct rectangle_t {
    low_x: f32,
    low_y: f32,
    high_x: f32,
    high_y: f32,
}

#[no_mangle]
pub extern "C" fn init(filename: *const c_char) -> i32 {
    let file = File::open(read_lossy(filename)).unwrap();
    let reader = BufReader::new(file);

    let ref mut pvec = *POINTS.lock().unwrap();
    for line in reader.lines() {
        let cur_line = line.unwrap();
        let mut words = cur_line.split_whitespace();
        let fx:f32 = FromStr::from_str(words.next().unwrap()).unwrap();
        let fy:f32 = FromStr::from_str(words.next().unwrap()).unwrap();
        let r:u32 = FromStr::from_str(words.next().unwrap()).unwrap();
        let rp = RPoint {x: fx, y: fy, rank: r};
        pvec.push(rp);
    }

    let _ = rayon::initialize(Configuration::new());
    process(pvec);
    return 0;
}

#[no_mangle]
pub extern "C" fn run(rect_p: *const rectangle_t, rect_len: usize) -> i32{
    let rectangles = unsafe {std::slice::from_raw_parts(rect_p, rect_len)};
    let mut rec_vec = Vec::new();
    for rectangle in rectangles{
        let lowrp = RPoint {x: rectangle.low_x, y: rectangle.low_y, rank: 0};
        let highrp = RPoint {x: rectangle.high_x, y: rectangle.high_y, rank:0};
        let brect = BoundingRect::from_corners(&lowrp, &highrp);
        rec_vec.push(brect);
    }
    find_solutions(rec_vec);
    return 0;
}

#[no_mangle]
pub extern "C" fn results(results: *mut c_char){
    let ref res_vec = *RESULTS.lock().unwrap();
    let mut obuf = String::with_capacity(12000000);
    for &(_, ref res) in res_vec{
        output(res.as_slice(), &mut obuf)
    }
    println!("{}", obuf);
    let raw_bytes = obuf.as_bytes();
    let len = obuf.len() + 1;
    let cstr = CString::new(raw_bytes).unwrap();
    let raw_cstr = cstr.as_ptr();
    unsafe {
        ptr::copy(raw_cstr, results, len);
    }
}

// Misc
fn read_lossy(cstr: *const c_char) -> String {
    unsafe {
        CStr::from_ptr(cstr).to_string_lossy().into_owned()
    }
}

fn output(xs: &[u32], obuf: &mut String) {
    for x in xs {
        let _ = write!(obuf, "{} ", x);
    }
    let _ = writeln!(obuf, "");
}
