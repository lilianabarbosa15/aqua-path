import { routeCell } from './main.js';

export async function boatInFountain() {
  try {
    const port = await navigator.serial.requestPort();
    await port.open({ baudRate: 112500 });

    const textDecoder = new TextDecoderStream();
    const readableStreamClosed = port.readable.pipeTo(textDecoder.writable);
    const inputStream = textDecoder.readable;
    const reader = inputStream.getReader();

    let buffer = "";

    const coordDisplay = document.getElementById("coord-received");

    while (true) {
      const { value, done } = await reader.read();
      if (done) break;
      if (value) {
        buffer += value;

        // Search for messages in the buffer
        let startIdx = buffer.indexOf("(");
        let endIdx = buffer.indexOf(")");

        while (startIdx !== -1 && endIdx > startIdx) {
          const rawMessage = buffer.substring(startIdx + 1, endIdx); // extract message without parentheses
          buffer = buffer.slice(endIdx + 1); // remove processed message from buffer

          // Process the message  --->  example: "x,y" -> "10,20"
          const parts = rawMessage.split(",");
          const x = parseInt(parts[0]);
          const y = parseInt(parts[1]);

          coordDisplay.textContent = `(${x}, ${y})`;
          routeCell(x, y);  // Mark the cell in the fountain

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
