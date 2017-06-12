use rayon::prelude::*;
use rpoint::RPoint;
use rtree::{RTree, BoundingRect};
use std::cmp::min;
use itertools::*;
use POINTS;
use RTREES;
use RESULTS;
use NUM_POINTS;

// If number of points less than threshold,
// use linear search
const THRESHOLD: usize = 3048;

pub fn process(pvec: &mut Vec<RPoint>){
    let mut trees = RTREES.lock().unwrap();
    let p_len = pvec.len();
    pvec.sort_by_key(|a| a.rank);

    let boundaries = iterate(THRESHOLD, |&i| i * 4)
        .take_while(|&i| i/4 < p_len).collect::<Vec<_>>();

    let construct_tree = |chunk| {
        let mut tree = RTree::new();
        let (&startc, &endc) = chunk;
        if pvec.len() < startc { return tree; }
        let endp = min(p_len, endc);
        for p in pvec[startc..endp].iter(){
            tree.insert(*p);
        }
        tree.fix_ranks();
        tree
    };

    *trees = zip(&boundaries, &boundaries[1..])
        .map(construct_tree).collect::<Vec<_>>();
}

pub fn find_solutions(rec_vec: Vec<BoundingRect<RPoint>>){
    let ref pvec = *POINTS.lock().unwrap();
    let mut results = RESULTS.lock().unwrap();
    let trees = RTREES.lock().unwrap();

    let rect_search = |(rec_idx, brect): (usize, &BoundingRect<RPoint>)| {
        let mut rankedpoints = Vec::with_capacity(40);
        let lower = brect.lower();
        let upper = brect.upper();

        let mut len = 0;
        for p in pvec[0..THRESHOLD].iter(){
            if p.x >= lower.x && p.y >= lower.y
                && p.x < upper.x && p.y < upper.y {
                    rankedpoints.push(p.rank);
                    len += 1;
                }
            if len == NUM_POINTS {
                return (rec_idx, rankedpoints);
            }
        }

        for tree in trees.iter(){
            let ps = tree.lookup_in_rectangle(&brect, 20 - rankedpoints.len());
            if ps.len() == 0 { continue; }
            rankedpoints.extend(ps);
            if rankedpoints.len() >= NUM_POINTS {
                return (rec_idx,
                        rankedpoints[0..NUM_POINTS].to_vec());
            }
        }
        return (rec_idx, rankedpoints);
    };

    *results = rec_vec
        .par_iter()
        .enumerate()
        .map(rect_search)
        .collect::<Vec<_>>();

    results.sort_by_key(|&(x,_)| x);
}
