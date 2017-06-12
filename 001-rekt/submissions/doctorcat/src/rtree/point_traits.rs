// Copyright 2017 The Spade Developers.
//
// Licensed under the Apache License, Version 2.0 <LICENSE-APACHE or
// http://www.apache.org/licenses/LICENSE-2.0> or the MIT license
// <LICENSE-MIT or http://opensource.org/licenses/MIT>, at your
// option. This file may not be copied, modified, or distributed
// except according to those terms.
use std::fmt::Debug;
use rtree::traits::SpadeNum;
use rtree::num::{zero};
use rtree::misc::{min_inline, max_inline};

/// Abstraction over a point with a fixed number of dimensions.
///
/// Spade will work with any point type implementing this trait, at the
/// moment points of the `cgmath` and `nalgebra` crates are supported.
/// Also, the trait is implemented for fixed arrays of length 2, 3 and 4, allowing
/// to use spade's datastructures with fixed size arrays as point coordinates.
/// That means that the trait's methods are also implemented for
/// these array types, thus be careful when importing `PointN`.
///
/// Implement this if you want spade to support your own point types.
/// Also, an empty implementation `TwoDimensional` or `ThreeDimensional`
/// should be given if appropriate.
pub trait PointN
    where Self: Clone,
          Self: Debug,
          Self: PartialEq {
    /// The points's internal scalar type.
    type Scalar: SpadeNum;

    /// Priority of object
    fn rank(&self) -> u32;

    /// The (fixed) number of dimensions of this point type.
    fn dimensions() -> usize;

    /// Creates a new point with all compoenents set to a certain value.
    fn from_value(value: Self::Scalar) -> Self;

    /// Returns the nth element of this point.
    fn nth(&self, index: usize) -> &Self::Scalar;
    /// Returns a mutable reference to the nth element of this point.
    fn nth_mut(&mut self, index: usize) -> &mut Self::Scalar;
}

/// Adds some private methods to the `PointN` trait.
pub trait PointNExtensions : PointN {
    /// Creates a new point with all components initialized to zero.
    fn new() -> Self {
        Self::from_value(zero())
    }

    /// Adds two points.
    fn add(&self, rhs: &Self) -> Self {
        self.component_wise(rhs, |l, r| l + r)
    }

    /// Substracts two points.
    fn sub(&self, rhs: &Self) -> Self {
        self.component_wise(rhs, |l, r| l - r)
    }

    /// Divides this point with a scalar value.
    fn div(&self, scalar: Self::Scalar) -> Self {
        self.map(|x| x / scalar.clone())
    }


    /// Multiplies this point with a scalar value.
    fn mul(&self, scalar: Self::Scalar) -> Self {
        self.map(|x| x * scalar.clone())
    }

    /// Applies a binary operation component wise.
    fn component_wise<F: Fn(Self::Scalar, Self::Scalar) -> Self::Scalar>(&self, rhs: &Self, f: F) -> Self {
        let mut result = self.clone();
        for i in 0 .. Self::dimensions() {
            *result.nth_mut(i) = f(self.nth(i).clone(), rhs.nth(i).clone());
        }
        result
    }

    /// Maps an unary operation to all compoenents.
    fn map<F: Fn(Self::Scalar) -> O::Scalar, O: PointN>(&self, f: F) -> O {
        let mut result = O::new();
        for i in 0 .. Self::dimensions() {
            *result.nth_mut(i)  = f(self.nth(i).clone());
        }
        result
    }

    /// Returns a new point containing the minimum values of this and another point (componentwise)
    fn min_point(&self, rhs: &Self) -> Self {
        self.component_wise(rhs, |l, r| min_inline(l, r))
    }

    /// Returns a new point containing the maximum values of this and another point (componentwise)
    fn max_point(&self, rhs: &Self) -> Self {
        self.component_wise(rhs, |l, r| max_inline(l, r))
    }

    /// Fold operation over all point components.
    fn fold<T, F: Fn(T, Self::Scalar) -> T>(&self, mut acc: T, f: F) -> T {
        for i in 0 .. Self::dimensions() {
            acc = f(acc, self.nth(i).clone());
        }
        acc
    }

    /// Checks if a property holds for all components of this and another point.
    fn all_comp_wise<F: Fn(Self::Scalar, Self::Scalar) -> bool>(&self, rhs: &Self, f: F) -> bool {
        for i in 0 .. Self::dimensions() {
            if !f(self.nth(i).clone(), rhs.nth(i).clone()) {
                return false;
            }
        }
        true
    }

    /// Returns the point's dot product.
    fn dot(&self, rhs: &Self) -> Self::Scalar {
        self.component_wise(rhs, |l, r| l * r).fold(zero(), |acc, val| acc + val)
    }

    /// Returns the point's squared length.
    fn length2(&self) -> Self::Scalar {
        self.dot(&self)
    }
}

impl <T> PointNExtensions for T where T: PointN { }

/// A two dimensional Point.
///
/// Some datastructures will only work if two dimensional points are given,
/// this trait makes sure that only such points can be passed.
pub trait TwoDimensional : PointN { }

