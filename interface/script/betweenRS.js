import { sentIntructions } from './sent.js';

// functions between final point and real boat localization
export function evaluateCoordenates(x, y, orientation, selectedCell) {
    if (!selectedCell || (isNaN(x) || isNaN(y))) {
      console.log(`No cell selected or No valid coordinates received.`);
      return;
    }

    // Coordinates of the final point
    const selectedCol = parseInt(selectedCell.dataset.col);
    const selectedRow = parseInt(selectedCell.dataset.row);
    
    // Distance between real localization and the final point
    const diffX = Math.abs(x - selectedCol);
    const diffY = Math.abs(y - selectedRow);
    
    // Difference between final point and boat coordinates
    const coordDiffDisplay = document.getElementById("coord-diff");
    coordDiffDisplay.textContent = `(${diffX}, ${diffY})`;

    // Calculate the kind of instructions to send
    sentIntructions({x, y, orientation}, selectedCell);
}