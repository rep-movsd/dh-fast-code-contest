// Copyright 2017 The Spade Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

extern crate num;
extern crate smallvec;
extern crate itertools;

mod traits;
mod point_traits;
mod boundingvolume;
mod misc;

use std::sync::Arc;
use self::traits::{SpatialObject};
use self::point_traits::{PointNExtensions};
use self::num::{zero};
use std::iter::Once;
use std::fmt;
use std::u32;
use self::smallvec::SmallVec;

pub use self::traits::*;
pub use self::boundingvolume::{BoundingRect};
pub use self::point_traits::{PointN, TwoDimensional};

#[doc(hidden)]
#[derive(Eq, PartialEq, Clone, Debug)]
pub struct RTreeOptions {
    max_size: usize,
    min_size: usize,
    reinsertion_count: usize,
}

impl Default for RTreeOptions {
    fn default() -> RTreeOptions {
        RTreeOptions::new()
    }
}

#[doc(hidden)]
impl RTreeOptions {
    pub fn new() -> RTreeOptions {
        RTreeOptions {
            max_size: 30,
            min_size: 15,
            reinsertion_count: 5,
        }
    }
}

/// Iterates over all entries in an r-tree.
/// Returned by `RTree::iter()`
pub struct RTreeIterator<'a, T>
    where T: SpatialObject + 'a {
    data: &'a DirectoryNodeData<T>,
    cur_index: usize,
    cur_iterator: Option<Box<RTreeNodeIterator<'a, T>>>,
}

#[allow(missing_docs)]
    pub enum RTreeNodeIterator<'a, T>
    where T: SpatialObject + 'a {
        LeafIterator(Once<&'a T>),
        DirectoryNodeIterator(RTreeIterator<'a, T>),
    }

impl <'a, T> RTreeIterator<'a, T>
    where T: SpatialObject {
    fn new(data: &'a DirectoryNodeData<T>) -> RTreeIterator<'a, T> {
        RTreeIterator {
            data: data,
            cur_index: 0,
            cur_iterator: data.children.first().map(
                |child| Box::new(RTreeNodeIterator::new(child))),
        }
    }
}

impl <'a, T> Iterator for RTreeIterator<'a, T>
    where T: SpatialObject {
    type Item = &'a T;

    fn next(&mut self) -> Option<&'a T> {
        if let Some(mut cur_iterator) = self.cur_iterator.as_mut() {
            if let Some(next) = cur_iterator.next() {
                // Child iterator can still iterate
                Some(next)
            } else {
                loop {
                    // Change to the next child
                    self.cur_index += 1;
                    if let Some(child_node) = self.data.children.get(self.cur_index) {
                        // Set a new iterator...
                        *cur_iterator = Box::new(RTreeNodeIterator::new(child_node));
                        // ... and call it
                        let next = cur_iterator.next();
                        if next.is_some() {
                            return next;
                        }
                    } else {
                        // We've iterated through all of our children
                        return None;
                    }
                }
            }
        } else {
            None
        }
    }
}

impl <'a, T> RTreeNodeIterator<'a, T>
    where T: SpatialObject {

    fn new(node: &'a RTreeNode<T>) -> RTreeNodeIterator<'a, T> {
        use self::RTreeNodeIterator::{LeafIterator, DirectoryNodeIterator};
        match node {
            &RTreeNode::Leaf(ref b) => LeafIterator(::std::iter::once(b)),
            &RTreeNode::DirectoryNode(ref data) =>
                DirectoryNodeIterator(RTreeIterator::new(data)),
        }
    }
}

impl <'a, T> Iterator for RTreeNodeIterator<'a, T>
    where T: SpatialObject {
    type Item = &'a T;

    fn next(&mut self) -> Option<&'a T> {
        use self::RTreeNodeIterator::{LeafIterator, DirectoryNodeIterator};
        match self {
            &mut LeafIterator(ref mut once) => once.next(),
            &mut DirectoryNodeIterator(ref mut iter) => iter.next(),
        }
    }
}

#[doc(hidden)]
impl <T> DirectoryNodeData<T>
    where T: SpatialObject {
    pub fn mbr(&self) -> BoundingRect<T::Point> {
        self.bounding_box.clone().unwrap()
    }

    fn new(depth: usize, options: Arc<RTreeOptions>) -> DirectoryNodeData<T> {
        DirectoryNodeData {
            bounding_box: None,
            children: Box::new(Vec::with_capacity(options.max_size + 1)),
            options: options,
            depth: depth,
            rank: 0,
        }
    }

    fn new_parent(mut children: Box<Vec<RTreeNode<T>>>, depth: usize, options: Arc<RTreeOptions>
    ) -> DirectoryNodeData<T> {
        let missing = options.max_size + 1 - children.len();
        children.reserve_exact(missing);
        let mut result = DirectoryNodeData {
            bounding_box: None,
            children: children,
            depth: depth,
            options: options,
            rank: 0,
        };
        result.update_mbr();
        result
    }

    #[inline]
    fn update_mbr(&mut self) {
        if let Some(first) = self.children.first() {
            let mut new_mbr = first.mbr();
            for child in &self.children[1 .. ] {
                new_mbr.add_rect(&child.mbr());
            }
            self.bounding_box = Some(new_mbr);
        } else {
            self.bounding_box = None;
        }
    }

    #[inline]
    fn update_mbr_with_element(&mut self, element_bb: &BoundingRect<T::Point>) {
        if let Some(ref mut bb) = self.bounding_box {
            bb.add_rect(element_bb);
        }  else {
            self.bounding_box = Some(element_bb.clone());
        }
    }

    fn insert(&mut self, t: RTreeNode<T>, state: &mut InsertionState) -> InsertionResult<T> {
        // Adjust own mbr - the element will most likely become a child of this node
        self.update_mbr_with_element(&t.mbr());
        if t.depth() + 1 == self.depth {
            // Force insertion into this node
            self.add_children(vec![t]);
            return self.resolve_overflow(state);
        }
        let expand = {
            let mut follow = self.choose_subtree(&t);
            follow.insert(t, state)
        };
        match expand {
            InsertionResult::Split(child) => {
                // Insert into own list
                self.add_children(vec![child]);
                self.resolve_overflow(state)
            },
            result @ InsertionResult::Reinsert(_) => {
                // Reinsertion can shrink the mbr
                self.update_mbr();
                result
            },
            complete => complete,
        }
    }

    fn resolve_overflow(&mut self, state: &mut InsertionState) -> InsertionResult<T> {
        if self.children.len() > self.options.max_size {
            if state.did_reinsert(self.depth) {
                // We did already reinsert on that level - split this node
                let offsplit = self.split();
                InsertionResult::Split(offsplit)
            } else {
                // We didn't attempt to reinsert yet - give it a try
                state.mark_reinsertion(self.depth);
                let reinsertion_nodes = self.reinsert();
                InsertionResult::Reinsert(reinsertion_nodes)
            }
        } else {
            InsertionResult::Complete
        }
    }

    #[inline(never)]
    fn split(&mut self) -> RTreeNode<T> {
        let axis = self.get_split_axis();
        assert!(self.children.len() >= 2);
        // Sort along axis
        self.children.sort_by(|l, r| l.mbr().lower().nth(axis).partial_cmp(&r.mbr().lower().nth(axis)).unwrap());
        let mut best = (zero(), zero());
        let mut best_index = self.options.min_size;

        for k in self.options.min_size .. self.children.len() - self.options.min_size + 1 {
            let mut first_mbr = self.children[k - 1].mbr();
            let mut second_mbr = self.children[k].mbr();
            let (l, r) = self.children.split_at(k);
            for child in l {
                first_mbr.add_rect(&child.mbr());
            }
            for child in r {
                second_mbr.add_rect(&child.mbr());
            }

            let overlap_value = first_mbr.intersect(&second_mbr).area();
            let area_value = first_mbr.area() + second_mbr.area();
            let new_best = (overlap_value, area_value);
            if new_best < best || k == self.options.min_size{
                best = new_best;
                best_index = k;
            }
        }
        let offsplit = Box::new(self.children.split_off(best_index));
        let result = RTreeNode::DirectoryNode(DirectoryNodeData::new_parent(offsplit, self.depth,
                                                                            self.options.clone()));
        self.update_mbr();
        result
    }

    #[inline(never)]
    fn reinsert(&mut self) -> Vec<RTreeNode<T>> {
        let center = self.mbr().center();
        // Sort with increasing order so we can use Vec::split_off
        self.children.sort_by(|l, r| {
            let l_center = l.mbr().center();
            let r_center = r.mbr().center();
            l_center.sub(&center).length2().partial_cmp(&(r_center.sub(&center)).length2()).unwrap()
        });
        let num_children = self.children.len();
        let result = self.children.split_off(num_children - self.options.reinsertion_count);
        self.update_mbr();
        result
    }

    fn get_split_axis(&mut self) -> usize {
        let mut best_goodness = zero();
        let mut best_axis = 0;
        for axis in 0 .. T::Point::dimensions() {
            // Sort children along the current axis
            self.children.sort_by(|l, r| l.mbr().lower().nth(axis)
                                  .partial_cmp(&r.mbr().lower().nth(axis)).unwrap());
            for k in self.options.min_size .. self.children.len() - self.options.min_size + 1 {
                let mut first_mbr = self.children[k - 1].mbr();
                let mut second_mbr = self.children[k].mbr();
                let (l, r) = self.children.split_at(k);
                for child in l {
                    first_mbr.add_rect(&child.mbr());
                }
                for child in r {
                    second_mbr.add_rect(&child.mbr());
                }

                let margin_value = first_mbr.half_margin() + second_mbr.half_margin();
                if best_goodness > margin_value || axis == 0 {
                    best_axis = axis;
                    best_goodness = margin_value;
                }
            }
        }
        best_axis
    }

    fn choose_subtree(&mut self, node: &RTreeNode<T>) -> &mut DirectoryNodeData<T> {
        assert!(self.depth >= 2, "Cannot choose subtree on this level");
        let insertion_mbr = node.mbr();
        let mut inclusion_count = 0;
        let mut min_area = zero();
        let mut min_index = 0;
        let mut first = true;
        for (index, child) in self.children.iter().enumerate() {
            let mbr = child.mbr();
            if mbr.contains_rect(&insertion_mbr) {
                inclusion_count += 1;
                let area = mbr.area();
                if area < min_area || first {
                    min_area = area;
                    min_index = index;
                    first = false;
                }
            }
        }
        if inclusion_count == 0 {
            // No inclusion found, subtree depends on overlap and area increase
            let all_leaves = self.depth <= 2;
            let mut min = (zero(), zero(), zero());

            for (index, child1) in self.children.iter().enumerate() {
                let mbr = child1.mbr();
                let mut new_mbr = mbr.clone();
                new_mbr.add_rect(&insertion_mbr);
                let overlap_increase = if all_leaves {
                    // Calculate minimal overlap increase
                    let mut overlap: <T::Point as PointN>::Scalar = zero();
                    let mut new_overlap: <T::Point as PointN>::Scalar = zero();
                    for child2 in self.children.iter() {
                        if child1 as *const _ != child2 as *const _ {
                            let child_mbr = child2.mbr();
                            overlap = overlap.clone() + mbr.intersect(&child_mbr).area();
                            new_overlap = new_overlap.clone() + new_mbr.intersect(&child_mbr).area();
                        }
                    }
                    let overlap_increase = new_overlap - overlap;
                    overlap_increase
                } else {
                    // Don't calculate overlap increase if not all children are leaves
                    zero()
                };
                // Calculate area increase and area
                let area = new_mbr.area();
                let area_increase = area.clone() - mbr.area();
                let new_min = (overlap_increase, area_increase, area);
                if new_min < min || index == 0 {
                    min = new_min;
                    min_index = index;
                }
            }
        }
        if let RTreeNode::DirectoryNode(ref mut data) = self.children[min_index] {
            data
        } else {
            panic!("There must not be leaves on this depth")
        }
    }

    fn add_children(&mut self, mut new_children: Vec<RTreeNode<T>>) {
        if let &mut Some(ref mut bb) = &mut self.bounding_box {
            for child in &new_children {
                bb.add_rect(&child.mbr());
            }
            self.children.append(&mut new_children);
            return;
        }
        if let Some(first) = new_children.first() {
            let mut bb = first.mbr();
            for child in new_children.iter().skip(1) {
                bb.add_rect(&child.mbr());
            }
            self.bounding_box = Some(bb);
        }
        self.children.append(&mut new_children);
    }

    fn lookup_in_rectangle<'b>(&'b self, result: &mut SmallVec<[u32; 32]>,
                               query_rect: &BoundingRect<T::Point>, mut current_rank: &mut u32, elems: usize) {
        for child in self.children.iter()
            .filter(|c| c.mbr().intersects(query_rect)) {
                match child {
                    &RTreeNode::DirectoryNode(ref data) => data.lookup_in_rectangle(result, query_rect, &mut current_rank, elems),
                    &RTreeNode::Leaf(ref t) => {
                        if t.mbr().intersects(query_rect) {
                            result.push(t.rank());
                            if result.len() > elems {
                                result.sort_unstable();
                                result.pop();
                                *current_rank = result[elems - 1];
                            }
                        }
                    }
                }
            }
    }

    fn fix_ranks(&mut self) {
        for child in self.children.iter_mut(){
            match child {
                &mut RTreeNode::DirectoryNode(ref mut data) => data.fix_ranks(),
                &mut RTreeNode::Leaf(_) => {},
            }
        }
        let ranks = self.children.iter().map(|x| x.rank()).collect::<Vec<_>>();
        self.rank = itertools::min(ranks).unwrap();
        self.children.sort_by_key(|x| x.rank());
    }
}

enum InsertionResult<T>
    where T: SpatialObject {
    Complete,
    Split(RTreeNode<T>),
    Reinsert(Vec<RTreeNode<T>>),
}

struct InsertionState {
    reinsertions: Vec<bool>,
}

impl InsertionState {
    fn new(max_depth: usize) -> InsertionState {
        let mut reinsertions = Vec::with_capacity(max_depth + 1);
        reinsertions.resize(max_depth, false);
        InsertionState {
            reinsertions: reinsertions,
        }
    }

    fn did_reinsert(&self, depth: usize) -> bool {
        self.reinsertions[depth]
    }

    fn mark_reinsertion(&mut self, depth: usize) {
        self.reinsertions[depth] = true;
    }
}

#[doc(hidden)]
impl <T> RTreeNode<T>
    where T: SpatialObject {
    pub fn depth(&self) -> usize {
        match self {
            &RTreeNode::DirectoryNode(ref data) => data.depth,
            _ => 0
        }
    }

    pub fn mbr(&self) -> BoundingRect<T::Point> {
        match self {
            &RTreeNode::DirectoryNode(ref data) => data.bounding_box.clone().unwrap(),
            &RTreeNode::Leaf(ref t) => t.mbr(),
        }
    }

    #[inline]
    pub fn rank(&self) -> u32 {
        match self {
            &RTreeNode::DirectoryNode(ref data) => data.rank,
            &RTreeNode::Leaf(ref t) => t.rank(),
        }
    }
}

#[doc(hidden)]
#[derive(Clone, Debug)]
pub struct DirectoryNodeData<T>
    where T: SpatialObject {
    bounding_box: Option<BoundingRect<T::Point>>,
    children: Box<Vec<RTreeNode<T>>>,
    depth: usize,
    options: Arc<RTreeOptions>,
    rank: u32,
}

#[doc(hidden)]
    #[derive(Clone, Debug)]
    pub enum RTreeNode<T>
    where T: SpatialObject {
        Leaf(T),
        DirectoryNode(DirectoryNodeData<T>),
    }

#[derive(Clone, Debug)]
    pub struct RTree<T> where T: SpatialObject {
        root: DirectoryNodeData<T>,
        size: usize,
    }

impl<T> Default for RTree<T> where T: SpatialObject + fmt::Debug {
        fn default() -> RTree<T> {
            RTree::new()
        }
    }

impl<T> RTree<T>
    where T: SpatialObject + fmt::Debug {
    /// Creates an empty r*-tree.
    pub fn new() -> RTree<T> {
        RTree::new_with_options(Default::default())
    }

    #[doc(hidden)]
    pub fn new_with_options(options: RTreeOptions) -> RTree<T> {
        let options = Arc::new(options);
        RTree {
            root: DirectoryNodeData::new(1, options),
            size: 0,
        }
    }

    /// Returns all objects (partially) contained in a rectangle
    pub fn lookup_in_rectangle(&self, query_rect: &BoundingRect<T::Point>, elems: usize) -> SmallVec<[u32; 32]> {
        let mut result = SmallVec::<[u32; 32]>::new();
        if self.size > 0 {
            self.root.lookup_in_rectangle(&mut result, query_rect, &mut u32::MAX, elems);
        }
        result.sort_unstable();
        result
    }

    pub fn fix_ranks(&mut self){
        if self.size > 0 {
            self.root.fix_ranks();
        }
    }
}

impl<T> RTree<T>
    where T: SpatialObject {
    /// Inserts a new element into the tree.
    ///
    /// This will require `O(log(n))` operations on average, where n is the number of
    /// elements contained in the tree.
    pub fn insert(&mut self, t: T) {
        let mut state = InsertionState::new(self.root.depth + 1);
        let mut insertion_stack = vec![RTreeNode::Leaf(t)];
        loop {
            if let Some(next) = insertion_stack.pop() {
                match self.root.insert(next, &mut state) {
                    InsertionResult::Split(node) => {
                        // The root node was split, create a new root and increase depth
                        let new_depth = self.root.depth + 1;
                        let options = self.root.options.clone();
                        let old_root = ::std::mem::replace(
                            &mut self.root, DirectoryNodeData::new(
                                new_depth, options));
                        self.root.add_children(vec![RTreeNode::DirectoryNode(old_root), node]);
                    },
                    InsertionResult::Reinsert(nodes) => {
                        // Schedule elements for reinsertion
                        insertion_stack.extend(nodes);
                    },
                    _ => {},
                }
            } else {
                break;
            }
        }
        self.size += 1;
    }
}
