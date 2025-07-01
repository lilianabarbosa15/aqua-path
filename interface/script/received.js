import { routeCell, selectedCell } from './main.js';
import { grid, orientationDeg } from './grid.js';
import { evaluateCoordenates } from './betweenRS.js';


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
// This function listens to the serial port for incoming messages from the boat
export let writer = null;
export async function boatSignals() {
  try {
    const serialPort = await navigator.serial.requestPort();
    await serialPort.open({ baudRate: 112500 });

    // Setup writer
    writer = serialPort.writable.getWriter();

    const textDecoder = new TextDecoderStream();
    const readableStreamClosed = serialPort.readable.pipeTo(textDecoder.writable);
    const inputStream = textDecoder.readable;
    const reader = inputStream.getReader();

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
        buffer += value;

        // Search for messages in the buffer
        let startIdx = buffer.indexOf("(");
        let endIdx = buffer.indexOf(")");

        while (startIdx !== -1 && endIdx > startIdx) {
          const rawMessage = buffer.substring(startIdx + 1, endIdx);  // extract message without parentheses
          buffer = buffer.slice(endIdx + 1);                          // remove processed message from buffer

          // Process the message
          const parts = rawMessage.split(",");
          const latitude = parseFloat(parts[0]);
          const longitude = parseFloat(parts[1]);
          const orientation = parseFloat(parts[2]);

          // Boat signals to coordinates
          const { x, y } = signalsToCoordinates({ lat: latitude, lon: longitude });
          routeCell(x, y);                                                  // mark the cell in the fountain
          coordDisplay.textContent = `(${x}, ${y})`;                        // display the coordinates
          const degree = orientationDeg[orientation]["deg"];
          orientationImagen.style.transform = `rotate(${degree}deg)`;       // change the rotation of the image

          // Update the display elements
          latitudeDisplay.textContent = `${latitude}`;
          longitudeDisplay.textContent = `${longitude}`;
          orientationDisplay.textContent = `${orientation} (${orientationDeg[orientation]["name"]})`;

          // Operations between coordenates
          evaluateCoordenates(x, y, orientation, selectedCell);

          // Search for the next message
          startIdx = buffer.indexOf("(");
          endIdx = buffer.indexOf(")");
        }
      }
    }
  } catch (error) {
    console.error("Error with COM:", error);
  }
}
