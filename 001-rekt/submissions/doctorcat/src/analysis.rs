use cogset::{Kmeans, Euclid};
use rpoint::RPoint;

pub fn clusters(ps: &Vec<RPoint>){
    let euclidpoints = ps.iter().map(|p| Euclid([p.x as f64, p.y as f64])).collect::<Vec<_>>();
    let kmeans = Kmeans::new(euclidpoints.as_slice(), 50);
    println!("{:?}", kmeans.clusters());
}
