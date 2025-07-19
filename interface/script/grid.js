// function to check if a point is inside a polygon
// This function uses the ray-casting algorithm to determine if a point is inside a polygon
export function pointInPolygon([x, y], poly) {
  let inside = false;
  for (let i = 0, j = poly.length - 1; i < poly.length; j = i++) {
    const [xi, yi] = poly[i];
    const [xj, yj] = poly[j];
    const intersect =
      yi > y !== yj > y &&
      x < ((xj - xi) * (y - yi)) / (yj - yi + 0.000001) + xi;
    if (intersect) inside = !inside;
  }
  return inside;
}

// The following code can be used to load the grid above dynamically
// "../grid_coordinates.csv" should contain the grid coordinates in the format: row,col,lat,lon
let grid = [];

export const loadGrid = async () => {
  const response = await fetch('../grid_coordinates.csv');
  const csv = await response.text();
  const lineas = csv.trim().split('\n').slice(1);
  grid = lineas.map(linea => {
    const [row, col, lat, lon] = linea.split(',');
    return {
      row: parseInt(row),
      col: parseInt(col),
      lat: parseFloat(lat),
      lon: parseFloat(lon)
    };
  });
};

export const getGrid = () => grid;

// Equivalence of each orientation
/*
  north_dir = -95deg:
  | 1: North (transform: rotate(-95deg)) | 2: Northeast (transform: rotate(-50deg)) |
  | 3: East  (transform: rotate(-5deg))  | 4: Southeast (transform: rotate(40deg))  |
  | 5: South   (transform: rotate(85deg))| 6: Southwest (transform: rotate(130deg)) |
  | 7: West (transform: rotate(-185deg)) | 8: Northwest (transform: rotate(-140deg))|
*/
const north_dir = -95;
export const orientationDeg = { 
  "1": { "deg": north_dir, "name": "North" }, 
  "2": { "deg": north_dir + 45, "name": "Northeast" },
  "3": { "deg": north_dir + 90, "name": "East" },
  "4": { "deg": north_dir + 135, "name": "Southeast" },
  "5": { "deg": north_dir + 180, "name": "South" },
  "6": { "deg": north_dir + 225, "name": "Southwest" },
  "7": { "deg": north_dir - 90, "name": "West" },
  "8": { "deg": north_dir - 45, "name": "Northwest" },
};
