import { routeCell, selectedCell } from './main.js';
import { loadGrid, getGrid, orientationDeg } from './grid.js';
import { evaluateCoordenates } from './betweenRS.js';

await loadGrid();
const grid = getGrid();
console.log(grid);

// Function to find the nearest cell in the grid based on GPS coordinates
function findNearestCell(target, grid) {
    let minDist = Infinity;
    let nearest = null;

    for (const cell of grid) {
        const dLat = cell.lat - target.lat;
        const dLon = cell.lon - target.lon;
        const dist = Math.sqrt(dLat * dLat + dLon * dLon);

        if (dist < minDist) {
            minDist = dist;
            nearest = { row: cell.row, col: cell.col };
        }
    }

    return nearest;
}

// Functions to convert latitude and longitude signals to coordinates
function signalsToCoordinates(pointGPS) {
  const cell = findNearestCell(pointGPS, grid);
  console.log(`Nearest cell found: (${cell.col}, ${cell.row}) for GPS point (${pointGPS.lat}, ${pointGPS.lon})`);
  return { x: cell.col, y: cell.row };
}

// Function to handle boat signals and convert them to coordinates
export let writer = null;
export async function boatSignals() {
  try {
    const serialPort = await navigator.serial.requestPort();
    await serialPort.open({ baudRate: 9600 });

    console.log("Port opened:", serialPort.getInfo());

    // Setup writer
    writer = serialPort.writable.getWriter();

    const reader = serialPort.readable.getReader();
    const decoder = new TextDecoder(); // decode manually each chunk
    let buffer = "";

    const coordDisplay = document.getElementById("coord-received");
    const latitudeDisplay = document.getElementById("lat-received");
    const longitudeDisplay = document.getElementById("lon-received");
    const orientationDisplay = document.getElementById("ori-received");
    const orientationImagen = document.getElementById("ori-received-img");

    while (true) {
      const { value, done } = await reader.read();
      if (done) break;
      if (value) {
        const chunk = decoder.decode(value);
        buffer += chunk;

        //console.log("Raw read:", chunk); // DEBUG: shows real-time chunk

        // Process complete messages (delimited by parentheses)
        let startIdx = buffer.indexOf("(");
        let endIdx = buffer.indexOf(")");

        while (startIdx !== -1 && endIdx > startIdx) {
          const rawMessage = buffer.substring(startIdx + 1, endIdx);
          buffer = buffer.slice(endIdx + 1); // keep rest of buffer

          const parts = rawMessage.split(",");
          const latitude = parseFloat(parts[0]);
          const longitude = parseFloat(parts[1]);
          const orientation = parseFloat(parts[2]);

          console.log('latitude: ', latitude);
          console.log('longitude: ', longitude);
          console.log('orientation: ', orientation);

          const { x, y } = signalsToCoordinates({ lat: latitude, lon: longitude });
          routeCell(x, y);
          coordDisplay.textContent = `(${x}, ${y})`;

          const degree = orientationDeg[orientation]["deg"];
          orientationImagen.style.transform = `rotate(${degree}deg)`;

          latitudeDisplay.textContent = `${latitude}`;
          longitudeDisplay.textContent = `${longitude}`;
          orientationDisplay.textContent = `${orientation} (${orientationDeg[orientation]["name"]})`;

          evaluateCoordenates(x, y, orientation, selectedCell);

          // Look for next message
          startIdx = buffer.indexOf("(");
          endIdx = buffer.indexOf(")");
        }
      }
    }
  } catch (error) {
    console.error("Error with COM:", error);
  }
}
