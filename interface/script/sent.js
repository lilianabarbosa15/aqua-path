import { orientationDeg } from './grid.js';
import { writer } from './received.js';

// Function to calculate the angle the boat should take based on magnetic north (always clockwise)
function calculateTargetAngle(origin, target) {
  // Calculate difference in X and Y between the target and the origin
  const deltaX = target.x - origin.x;
  const deltaY = target.y - origin.y;

  // Compute the angle (in radians) from the positive X-axis to the vector (deltaX, deltaY)
  // Negated to match clockwise compass rotation
  const angleRad = - Math.atan2(deltaY, deltaX);

  // Convert angle from radians to degrees
  const angleDeg = (angleRad * (180 / Math.PI)) - 90;   // shift by -90° so the arrow go from MUUA to B16

  // Adjust to compass orientation:
  // - Add real magnetic north offset from compass calibration
  // - Normalize to [0, 360) using modulo
  const compassAngle = ( - orientationDeg["1"].deg - angleDeg + 360) % 360;

  console.log(`calculateTargetAngle: (mathDeg: ${angleDeg}, compassDeg: ${compassAngle})`);
  return compassAngle;
}

/**
 * Get the closest cardinal direction (1–8) to a given compass angle
 * @param {number} compassAngle - Angle in degrees [0, 360)
 * @returns {Object} - The closest direction object {deg, name}
 */
function getClosestCardinalDirection(compassAngle) {
  let closestKey = null;
  let minDiff = Infinity;

  for (const key in orientationDeg) {
    const dir = orientationDeg[key];
    const dir_value = dir.deg - orientationDeg[1]["deg"];
    const diff = Math.abs(((compassAngle - dir_value + 540) % 360) - 180); // shortest angular distance
    console.log(`dir: ${dir.name}, diff: ${diff}`);
    if (diff < minDiff) {
      minDiff = diff;
      closestKey = key;
    }
  }

  console.log(`calculateTargetAngle: (key: ${closestKey}, compassDeg: ${compassAngle}, closestCardinalDirection: ${orientationDeg[closestKey]["name"]})`);
  return closestKey;
}

/**
 * Sends navigation instructions (orientation and speed) to the boat via serial port.
 * 
 * @param {number|string} orientationKey - The direction index (1 to 8) to send to the boat.
 * @param {number} speed - The speed value to send.
 */
async function sendInstructionsToBoat(orientationKey, speed) {
  // Check if the serial writer is initialized
  if (!writer) {
    console.error("Serial writer not initialized.");
    return;
  }

  // Format the message to match the expected structure: (orientation,speed)
  const message = `(${orientationKey},${speed})`;
  console.log("Sending:", message);

  // Encode the message as a Uint8Array so it can be written to the serial port
  const encoder = new TextEncoder();
  await writer.write(encoder.encode(message));  // Send the encoded message
}

// This function calculates everything necessary to drive the boat
const orientationDisplay = document.getElementById("ori-sent");
const orientationImagen = document.getElementById("ori-sent-img");
const velocity = document.getElementById("vel-sent");

export function calculateIntructions(boatCell, objectifCell) {
    const x = objectifCell.dataset.col;
    const y = objectifCell.dataset.row;

    const compassAngle = calculateTargetAngle(boatCell, {x, y});
    const orientation = getClosestCardinalDirection(compassAngle);

    // Update orientation display
    const degree = orientationDeg[orientation]["deg"];
    orientationImagen.style.transform = `rotate(${degree}deg)`;       // change the rotation of the image
    orientationDisplay.textContent = `${orientation} (${orientationDeg[orientation]["name"]})`;

    // Update velocity display
    const vel = 0.5;
    velocity.textContent = `${vel}`;

    sendInstructionsToBoat(orientation, vel);
}

