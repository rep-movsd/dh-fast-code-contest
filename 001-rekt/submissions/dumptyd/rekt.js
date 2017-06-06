let fs = require('fs');
let readline = require('readline');
let Heap = require('heap');
let pointsFile = 'points.txt', inputFile = 'rects.txt', resultFile = 'results.txt';


let coordinates = [], coordinatesSortedByRank = [];

let mipmap = [];

function readIOFiles() {
  let encoding = 'utf-8';

  let inputReaderPromise = new Promise( (resolve, reject) => {
    fs.readFile(inputFile, encoding, (err, data) => {
      if (err) 
        reject(err);
      else {
        let input = data.split('\n');
        input = input.slice(0, input.length-1).map(rekt => rekt.split(' ').map( x => Number(x)) );
        resolve(input);
      }
    });
  });

  let outputReaderPromise = new Promise( (resolve, reject) => {
    fs.readFile(resultFile, encoding, (err, data) => {
      if (err) 
        reject(err);
      else {
        let output = data.split('\n');
        output = output.slice(0, output.length-1);
        resolve(output);
      }
    });
  });

  return Promise.all([inputReaderPromise, outputReaderPromise]);
}

function compareOutputs(expected, actual) {
  let out = actual.join('\n') + '\n';
  fs.writeFile('out.txt', out, (err) => {
    if (err)
      throw err;
    else
      console.log('Done');
  });
}

function init(seed) {
  pointsFile = seed + pointsFile, inputFile = seed + inputFile, resultFile = seed + resultFile;
  console.log('reading...');
  let pointsReader = readline.createInterface({
    input: fs.createReadStream(pointsFile)
  });
  let input = [], expectedOutput = [];
  pointsReader.on('line', function (line) {
    let varArr = line.split(' ').map( x => Number(x) );
    let obj = {
      x: varArr[0],
      y: varArr[1],
      r: varArr[2]
    };
    coordinates.push(obj);
  });
  pointsReader.on('close', function(err) {
    if(err)
      throw err;
    console.log('processing...');
    let counter = 0;
    let chunks = [0, 3050, 9150, 27450, 82350, 247050, 741150, 2223450, 6670350, 20011050 ];
    let totalYet = chunks.reduce((old,elem) => old + elem, 0);
    let remaining = coordinates.length - totalYet;
    if (remaining > 0)
      chunks.push(remaining);
    coordinatesSortedByRank = coordinates.slice().sort( (a, b) => a.r - b.r );
    for (let i = 1; i < chunks.length; i++) {
    let last = false;
      let temparr = [];
      for (let j = chunks[i-1]; j < chunks[i]; j++) {
        if(coordinatesSortedByRank[j] === undefined || j >= coordinatesSortedByRank.length) {
          break;
        }
        temparr.push(coordinatesSortedByRank[j]);
      }
      if(temparr.length) {
        let coordinatesSortedByX = temparr.slice().sort( (a, b) => a.x - b.x );
        let coordinatesSortedByY = temparr.slice().sort( (a, b) => a.y - b.y );
        //console.log(temparr.length);
        mipmap.push({
          x: coordinatesSortedByX,
          y: coordinatesSortedByY
        });
      }
    }
    readIOFiles().then( ([inputRects, outputPoints]) => {
      input = inputRects;
      expectedOutput = outputPoints;
      console.log(`running on ${coordinates.length} points and ${input.length} rectangles...`);
      console.time('total');
      let actualOutput = input.slice().map( (rekt, i) => {
        //console.time('rekt ' + (i+1));
        //let toret = 
        //console.timeEnd('rekt ' + (i+1));
        return run(rekt);;
      });
      console.timeEnd('total');
      compareOutputs(expectedOutput, actualOutput);
    }).catch( err => {
      console.log('stuff went wrong here');
      throw err;
    });
  });
}

function binarySearch(elemToFind, arr, propToSearchBy) {
  let start = 0, end = arr.length-1;
  let mid = 0;
  let minIndex = -1;
  while (start <= end) {
    mid = Math.floor((start + end) / 2);
    minIndex = mid;
    if (arr[mid][propToSearchBy] == elemToFind) {
      return mid;
    }
    if(arr[mid][propToSearchBy] < elemToFind) {
      start = mid + 1;
    }
    if(arr[mid][propToSearchBy] > elemToFind) {
      end = mid - 1;
    }
  }
  if (arr[minIndex][propToSearchBy] < elemToFind) {
    for (let i = minIndex+1; i < arr.length; i++) {
      if(arr[i][propToSearchBy] >= elemToFind)
        return i; 
    }
    return arr.length;
  }
  return minIndex;
}

function run([x1, x2, y1, y2]) {
  let arr = [];
  for (let i = 0; i < mipmap[0].x.length; i++) {
    let pt = coordinatesSortedByRank[i]; 
    if(pt.y < y2 && pt.y >= y1 && pt.x < x2 && pt.x >= x1) {
      arr.push(pt.r);
    }
    if(arr.length == 20)
      break;
  }
  if (arr.length == 20) {
    let result = arr.reduce((val, next) => {
      return val + next + ' ';
    }, '');
    return result;
  }
  // console.log(x1, x2, y1, y2);
  // console.log(arr);
  for (let i = 1; i < mipmap.length; i++){
    let currMipmap = mipmap[i];
    // if(i==1) {
    //   currMipmap.x.forEach(boo => console.log(boo));
    // }
    let xStartIndex = binarySearch(x1, currMipmap.x, 'x');
    let xStopIndex = binarySearch(x2, currMipmap.x, 'x') ;
    let yStartIndex = binarySearch(y1, currMipmap.y, 'y');
    let yStopIndex = binarySearch(y2, currMipmap.y, 'y');
    let xPointsInside = currMipmap.x.slice(xStartIndex, xStopIndex);
    let yPointsInside = currMipmap.y.slice(yStartIndex, yStopIndex);
    let points = [];
    //console.log([x1,x2,y1,y2], currMipmap.x.length, xStartIndex,currMipmap.x[xStartIndex]);
    // while (currMipmap.x[xStartIndex] && currMipmap.x[xStartIndex].x < x2 && xStartIndex < currMipmap.x.length) {
    //   let p = currMipmap.x[xStartIndex];
    //   if(p.y < y2 && p.y >= y1)
    //     points.push(p.r);
    //   xStartIndex++;
    // }
    if(xPointsInside.length < yPointsInside.length) { 
      //console.log('in x');
      xPointsInside.forEach( p => {
        if(p.y < y2 && p.y >= y1)
          points.push(p.r);
      });
    }
    else {
      //console.log('in y');
      yPointsInside.forEach( p => {
        if(p.x < x2 && p.x >= x1)
          points.push(p.r);
      });
    }
    // points = [...points];
    points.sort((a,b) => a - b);
    arr = arr.concat(points);
    if (arr.length >= 20)
      break;

  }
  //arr.sort((a,b) => a-b);
  let result = arr.slice(0,20).reduce((val, next) => {
    return val + next + ' ';
  }, '');
  //console.timeEnd('misc');
  return result;

}
init(process.argv[2]);