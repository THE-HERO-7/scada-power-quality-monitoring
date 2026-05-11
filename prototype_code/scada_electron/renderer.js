const waveCtx = document.getElementById("wave").getContext("2d");
const fftCtx = document.getElementById("fft").getContext("2d");

// Helper to draw grid/scales
function drawScales(ctx, color) {
    ctx.strokeStyle = "#333";
    ctx.lineWidth = 1;
    // Horizontal Center Line
    ctx.beginPath();
    ctx.moveTo(0, 100); ctx.lineTo(600, 100);
    ctx.stroke();
    // Vertical Grid
    for(let i=0; i<600; i+=100) {
        ctx.beginPath(); ctx.moveTo(i, 0); ctx.lineTo(i, 200); ctx.stroke();
    }
}

function draw(ctx, data, color, scale, offset) {
    ctx.clearRect(0, 0, 600, 200);
    drawScales(ctx, color); // Draw background grid
    
    ctx.beginPath();
    ctx.strokeStyle = color;
    ctx.lineWidth = 2;
    data.forEach((v, i) => {
        let x = i * (600 / data.length);
        let y = offset - (v * scale);
        if (i === 0) ctx.moveTo(x, y);
        else ctx.lineTo(x, y);
    });
    ctx.stroke();
}

window.api.onSerial((line) => {
    const parts = line.split(",");
    
    // Check the FIRST item in the array
    if (parts[0] === "W") {
        const rawWave = parts.slice(1).map(Number);
        draw(waveCtx, rawWave, "cyan", 300, 100);
        const fft = computeDFT(rawWave);
        draw(fftCtx, fft, "orange", 2.0, 190);
    } 
    else if (parts[0] === "F") {
        document.getElementById("rms").innerText   = parts[1];
        document.getElementById("peak").innerText  = parts[2];
        document.getElementById("freq").innerText  = parts[3];
        document.getElementById("crest").innerText = parts[4];
    }
});


window.api.onBackend((msg) => {
    const p = msg.split(",");
    
    // Check if the message starts with STATUS
    if (p[0] === "STATUS") {
        const statusText = p[1]; // "NORMAL" or "ANOMALY"
        const thdVal = parseFloat(p[2]);
        const scoreVal = parseFloat(p[3]);

        // 1. Update THD Current
        const currentEl = document.getElementById("thd_c");
        if (currentEl) currentEl.innerText = thdVal.toFixed(4);

        // 2. Set THD Initial (Baseline) only once
        const initialEl = document.getElementById("thd_i");
        if (initialEl && (initialEl.innerText === "--" || initialEl.innerText === "0.0000")) {
            initialEl.innerText = thdVal.toFixed(4);
        }

        // 3. Update ML Score
        const scoreEl = document.getElementById("score");
        if (scoreEl) {
            scoreEl.innerText = scoreVal.toFixed(4);
            // Turn orange if score is getting low (approaching anomaly)
            scoreEl.style.color = scoreVal < 0 ? "#ff8c00" : "#00ff00";
        }

        // 4. Update Status Text and Color
        const statusEl = document.getElementById("status");
        if (statusEl) {
            statusEl.innerText = statusText === "ANOMALY" ? "🔴 ANOMALY" : "🟢 NORMAL";
            statusEl.style.color = statusText === "ANOMALY" ? "red" : "#00ff00";
        }
    }
});

