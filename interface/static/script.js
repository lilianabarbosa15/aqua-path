const grid = document.getElementById('grid');
let destino = null;
let actual = null;
let destinoAlcanzado = false;

// Crear la cuadrícula de 40x40 = 1600 celdas
for (let i = 0; i < 1600; i++) {
    const cell = document.createElement('div');
    cell.classList.add('cell');
    cell.dataset.index = i;
    cell.onclick = () => {
        const x = i % 40;
        const y = Math.floor(i / 40);
        destino = { x, y };
        destinoAlcanzado = false;  // reiniciar si cambian destino
        fetch('/enviar-destino', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(destino)
        });
        actualizar();
    };
    grid.appendChild(cell);
}

function actualizar() {
    // Limpiar todas las celdas
    document.querySelectorAll('.cell').forEach(cell => {
        cell.classList.remove('destino', 'actual');
    });
    
    // Pintar posición actual del barco (rojo)
    if (actual) {
        const idxActual = actual.y * 40 + actual.x;
        grid.children[idxActual]?.classList.add('actual');
    }

    // Pintar destino si aún no ha llegado (verde)
    if (destino) {
        const idxDestino = destino.y * 40 + destino.x;
        grid.children[idxDestino]?.classList.add('destino');

        // Detectar llegada
        if (
            actual &&
            actual.x === destino.x &&
            actual.y === destino.y &&
            !destinoAlcanzado
        ) {
            destinoAlcanzado = true;
            setTimeout(() => {
                destino = null;
                destinoAlcanzado = false;
                actualizar();
                alert("¡El barco ha llegado al destino!");
            }, 300); // espera corta para que se vea el cruce
        }
    }
}

// Obtener la posición actual del barco cada 0.5 s
setInterval(async () => {
    try {
        const res = await fetch('/posicion');
        const data = await res.json();
        actual = data;
        actualizar();
    } catch (e) {
        console.error("Error al obtener posición:", e);
    }
}, 500);
