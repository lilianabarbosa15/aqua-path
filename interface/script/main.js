// Others files
import { pointInPolygon } from './grid.js';
import { boatInFountain } from './received.js';


// Grid of the fountain ------------------------------------------------------------------------------------------------------------
// This script creates a grid of cells within a polygon defined by the SVG element with id "zone"
const svg = document.getElementById("map-svg");
const zone = document.getElementById("zone");

const polygonPoints = zone.getAttribute("points").trim().split(" ").map(p => p.split(',').map(Number));

const cols = 20;
const rows = 27;
const width = 708;
const height = 683;
const cellW = width / cols;
const cellH = height / rows;

let selectedCell = null; // global variable to store the selected cell

// Iterate over each cell
for (let r = 0; r < rows; r++) {
  for (let c = 0; c < cols; c++) {
    const x0 = c * cellW + 41;
    const y0 = r * cellH + 20;
    const x1 = x0 + cellW;
    const y1 = y0 + cellH;

    const cellPoints = [
      [x0, y0],
      [x1, y0],
      [x1, y1],
      [x0, y1],
    ];

    // Verified that all points are inside the polygon
    const allInside = cellPoints.every(p => pointInPolygon(p, polygonPoints));

    if (allInside) {
      const cell = document.createElementNS("http://www.w3.org/2000/svg", "polygon");
      cell.setAttribute("points", cellPoints.map(p => p.join(",")).join(" "));
      cell.setAttribute("fill", "rgba(0, 200, 255, 0.1)");
      cell.setAttribute("stroke", "white");
      cell.setAttribute("stroke-width", "0.25");
      cell.style.cursor = "pointer";
      cell.dataset.col = c;
      cell.dataset.row = r;

      cell.addEventListener("click", () => {
        // Restore previous cell color
        if (selectedCell) {
          selectedCell.setAttribute("fill", "rgba(0, 200, 255, 0.1)");
        }

        // Update new cell
        selectedCell = cell;
        const coordSent = document.getElementById('coord-sent');
        coordSent.textContent = `(${c}, ${r})`;
        cell.setAttribute("fill", "rgba(13, 255, 0, 0.39)");
      });

      svg.appendChild(cell);
    }
  }
}


// Handle the boat in fountain logic  ----------------------------------------------------------------------------------------------
let previousCol = null;
let previousRow = null;

export function routeCell(col, row) {
  // Restore previous cell color
  if (previousCol !== null && previousRow !== null) {
    const previousCell = svg.querySelector(`polygon[data-col="${previousCol}"][data-row="${previousRow}"]`);
    if (previousCell) {
      previousCell.setAttribute("fill", "rgba(0, 200, 255, 0.1)");
    }
  }

  // Update new cell
  const cell = svg.querySelector(`polygon[data-col="${col}"][data-row="${row}"]`);
  if (cell) {
    cell.setAttribute("fill", "rgba(176, 223, 255, 0.55)"); // update color to indicate routing
  }
  previousCol = col;
  previousRow = row;
}

document.getElementById("connect-btn").addEventListener("click", boatInFountain);   //////////////////////////// send instructions to the boat???


// Handle the decision to send the instructions to the boat ------------------------------------------------------------------------  ???????????
//document.getElementById("send-btn").addEventListener("click", () => { })