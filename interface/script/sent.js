import { orientationDeg } from './grid.js';


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

// 
export function sentIntructions(boatCell, objectifCell) {
    console.log(`SENT BOAT: (${boatCell.x}, ${boatCell.y}, ${boatCell.orientation})`);
    console.log(`SENT OBJECTIF: (${objectifCell.dataset.col}, ${objectifCell.dataset.row})`);

    const x = objectifCell.dataset.col;
    const y = objectifCell.dataset.row;
    calculateTargetAngle(boatCell, {x, y});
}

