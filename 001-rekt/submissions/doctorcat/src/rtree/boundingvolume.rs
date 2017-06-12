// Copyright 2017 The Spade Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.

use rtree::num::{zero, one};
use rtree::point_traits::{PointN, PointNExtensions};
use rtree::misc::max_inline;

/// An axis aligned minimal bounding rectangle.
///
/// An axis aligned minimal bounding rectangle is the smallest rectangle that completely
/// surrounds an object and is aligned along all axes. The vector type `V`'s dimension
/// determines if this is a rectangle, a box or a higher dimensional volume.
#[derive(Clone, PartialEq, Debug)]
pub struct BoundingRect<V: PointN> {
    lower: V,
    upper: V,
}

impl <V> BoundingRect<V> where V: PointN {

    /// Creates a bounding rectangle that contains exactly one point.
    ///
    /// This will create a bounding rectangle with `lower == upper == point`.
    pub fn from_point(point: V) -> BoundingRect<V> {
        BoundingRect {
            lower: point.clone(),
            upper: point,
        }
    }

    /// Creates a bounding rectangle that contains two points.
    pub fn from_corners(corner1: &V, corner2: &V) -> BoundingRect<V> {
        BoundingRect {
            lower: corner1.min_point(&corner2),
            upper: corner1.max_point(&corner2),
        }
    }

    /// Returns the lower corner of the bounding rectangle.
    ///
    /// The lower corner has the smaller coordinates.
    pub fn lower(&self) -> V {
        self.lower.clone()
    }

    /// Returns the upper corner of the bounding rectangle.
    ///
    /// The upper corner has the larger coordinates.
    pub fn upper(&self) -> V {
        self.upper.clone()
    }

    /// Checks if a point is contained within the bounding rectangle.
    ///
    /// A point lying exactly on the bounding rectangle's border is also contained.
    /// Checks if another bounding rectangle is completley contained withing this rectangle.
    ///
    /// A rectangle is contained if and only if all four corner are contained (see `contains_point`).
    #[inline]
    pub fn contains_rect(&self, rect: &BoundingRect<V>) -> bool {
        self.lower.all_comp_wise(&rect.lower(), |l, r| l <= r) &&
            self.upper.all_comp_wise(&rect.upper(), |l, r| l >= r)
    }

    /// Enlarges this bounding rectangle to contain a rectangle.
    ///
    /// If the rectangle is already contained, nothing will be changed.
    /// Otherwise, this will enlarge `self` to be just large enough
    /// to contain the new rectangle.
    #[inline]
    pub fn add_rect(&mut self, rect: &BoundingRect<V>) {
        self.lower = self.lower.min_point(&rect.lower);
        self.upper = self.upper.max_point(&rect.upper);
    }

    /// Returns the rectangle's area.
    pub fn area(&self) -> V::Scalar {
        let diag = self.upper().sub(&self.lower());
        diag.fold(one(), |acc, value| max_inline(acc * value, zero()))
    }

    /// Returns half of the rectangle's margin, thus `width + height`.
    pub fn half_margin(&self) -> V::Scalar {
        let diag = self.upper().sub(&self.lower());
        diag.fold(zero(), |acc, value| max_inline(acc + value, zero()))
    }

    /// Returns the rectangle's center.
    pub fn center(&self) -> V {
        let two = one::<V::Scalar>() + one::<V::Scalar>();
        self.lower().add(&(self.upper().sub(&self.lower()).div(two)))
    }

    /// Returns the intersection of this and another bounding rectangle.
    ///
    /// If the rectangles do not intersect, a bounding rectangle with an area and
    /// margin of zero is returned.
    pub fn intersect(&self, other: &BoundingRect<V>) -> BoundingRect<V> {
        BoundingRect {
            lower: self.lower.max_point(&other.lower),
            upper: self.upper.min_point(&other.upper),
        }
    }

    /// Returns true if this and another bounding rectangle intersect each other.
    /// If the rectangles just "touch" each other at one side, true is returned.
    #[inline]
    pub fn intersects(&self, other: &BoundingRect<V>) -> bool {
        self.lower.all_comp_wise(&other.upper(), |l, r| l < r) &&
            self.upper.all_comp_wise(&other.lower(), |l, r| l >= r)
    }

}
