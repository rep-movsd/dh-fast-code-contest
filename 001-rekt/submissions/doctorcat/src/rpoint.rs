#[cfg(test)]
extern crate quickcheck;

#[cfg(test)]
use quickcheck::Arbitrary;
use rtree::{TwoDimensional, PointN};
use std::fmt;

// Points
#[derive(Copy, Clone, Debug, PartialEq, PartialOrd)]
pub struct RPoint {
    pub x: f32,
    pub y: f32,
    pub rank: u32,
}

impl fmt::Display for RPoint {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "({} {}, {})", self.x, self.y, self.rank)
    }
}

impl PointN for RPoint where {
    type Scalar = f32;
    fn dimensions() -> usize {2}
    fn from_value(value: Self::Scalar) -> Self {
        return RPoint {x: value, y: value, rank: 0};
    }
    fn nth(&self, index: usize) -> &f32 {
        match index {
            0 => &self.x,
            1 => &self.y,
            _ => panic!("Index out of bounds."),
        }
    }
    fn nth_mut(&mut self, index: usize) -> &mut f32 {
        match index {
            0 => &mut self.x,
            1 => &mut self.y,
            _ => panic!("Index out of bounds."),
        }
    }
    #[inline]
    fn rank(&self) -> u32{
        self.rank
    }
}

impl TwoDimensional for RPoint {}

#[cfg(test)]
impl Arbitrary for RPoint {
    fn arbitrary<G: quickcheck::Gen>(g: &mut G) -> RPoint {
        let x = g.next_f32();
        let y = g.next_f32();
        let rank = g.next_u32();
        RPoint {
            x: x,
            y: y,
            rank: rank,
        }
    }
}
