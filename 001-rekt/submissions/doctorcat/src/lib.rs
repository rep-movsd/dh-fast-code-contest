#[macro_use]
extern crate lazy_static;
extern crate nalgebra as na;
extern crate num;
extern crate order_stat;
#[cfg(test)]
#[macro_use]
extern crate quickcheck;
extern crate rayon;
extern crate spade;


//mod rtree;
mod rpoint;
mod tree;

use rpoint::RPoint;
use std::cmp::min;
use std::ffi::{CStr, CString};
use std::fmt::Write;
use std::fs::File;
use std::io::BufReader;
use std::io::prelude::*;
use std::os::raw::c_char;
use std::ptr;
use std::str::FromStr;
use std::sync::Mutex;
use std::time::{Duration, SystemTime};
use spade::BoundingRect;
use spade::rtree::RTree;

lazy_static! {
    static ref TREE: Mutex<RTree<RPoint>> = Mutex::new(RTree::new());
    static ref POINTS: Mutex<Vec<RPoint>> = Mutex::new(Vec::with_capacity(1000000));
    static ref OUTPUT: Mutex<String> = Mutex::new(String::with_capacity(1048576));
}

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
    let mut lines = reader.lines();
    let mut tree = TREE.lock().unwrap();

    let ref mut pvec = *POINTS.lock().unwrap();
    for _i in 0..1000000 {
        let line = lines.next().unwrap().unwrap();
        let mut words = line.split_whitespace();
        let fx:f32 = FromStr::from_str(words.next().unwrap()).unwrap();
        let fy:f32 = FromStr::from_str(words.next().unwrap()).unwrap();
        let r:u32 = FromStr::from_str(words.next().unwrap()).unwrap();
        let rp = RPoint {x: fx, y: fy, rank: r};
        pvec.push(rp);
    }
    pvec.sort_by_key(|a| a.rank);

    for p in pvec{
        tree.insert(p.clone());
    }
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

    let ref pvec = *POINTS.lock().unwrap();
    let mut obuf = OUTPUT.lock().unwrap();
    let tree = TREE.lock().unwrap();

    for brect in rec_vec {
        //let now = SystemTime::now();
        let mut rankedpoints = Vec::with_capacity(20);
        let lower = brect.lower();
        let upper = brect.upper();
        let mut len = 0;

        'outer: for chunk in pvec.chunks(8192) {
            for p in chunk{
                if p.x >= lower.x && p.y >= lower.y
                    && p.x <= upper.x && p.y <= upper.y {
                    rankedpoints.push(p.clone());
                    len += 1;
                    }
                if len == 20 {
                    break 'outer;
                }
            }
            if len < 10 { break; }
        }

        // See tree.rs for WIP range tree to replace this tree
        if len == 20 {
            output(rankedpoints.as_slice(), &mut obuf);
        }
        else {
            let mut points = tree.lookup_in_rectangle(&brect);
            let p_len = points.len();
            if p_len > 0 {
                let lim1 = min(p_len, 20);
                let mut ps = points.clone();
                let mut pslice = ps.as_mut_slice();
                order_stat::kth_by(pslice, lim1 - 1, |x, y| x.rank.cmp(&y.rank));
                let mut twenty = Vec::with_capacity(20);
                for b in 0..lim1{
                    twenty.push(pslice[b].clone());
                }
                twenty.sort_by(|x, y| x.rank.cmp(&y.rank));
                output(twenty.as_slice(), &mut obuf);
            }
            else {
                writeln!(&mut obuf, "");
            }
        }
        // match now.elapsed() {
        //     Ok(elapsed) =>{
        //         println!("{}", elapsed.subsec_nanos());
        //     }
        //     _ => {
        //         println!("Time measurement error");
        //     }
        // }
    }
    print!("{}", *obuf);
    return 0;
}

#[no_mangle]
pub extern "C" fn results(results: *mut c_char){
    let obuf = OUTPUT.lock().unwrap();
    let raw_bytes = obuf.as_bytes();
    let len = obuf.len() + 1;
    let cstr = CString::new(raw_bytes).unwrap();
    let raw_cstr = cstr.as_ptr();
    unsafe {
        ptr::copy(raw_cstr, results, len);
    }
}

fn output(xs: &[RPoint], obuf: &mut String) {
    for x in xs {
        write!(obuf, "{} ", x.rank);
    }
    writeln!(obuf, "");
}

// Misc
fn read_lossy(cstr: *const c_char) -> String {
    unsafe {
        CStr::from_ptr(cstr).to_string_lossy().into_owned()
    }
}
