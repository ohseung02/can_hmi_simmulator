const wsUrl = "ws://localhost:18080/ws";
let ws;

const elSteeringWheel = document.getElementById('steering-wheel');
const valSteering = document.getElementById('val-steering');
const valBrake = document.getElementById('val-brake');
const brakeBar = document.getElementById('brake-bar');
const valThrottle = document.getElementById('val-throttle');
const throttleBar = document.getElementById('throttle-bar');

const valRpm = document.getElementById('val-rpm');
const valSpeed = document.getElementById('val-speed');
const valGear = document.getElementById('val-gear');
const valClimate = document.getElementById('val-climate');

const canIdInput = document.getElementById('can-id');
const canDataInput = document.getElementById('can-data');
const btnSend = document.getElementById('btn-send');
const btnReplayStart = document.getElementById('btn-replay-start');
const btnReplayStop = document.getElementById('btn-replay-stop');
const canLog = document.getElementById('can-log');
const connStatus = document.getElementById('conn-status');

let frameCount = 0;
let lastTime = performance.now();
const valFps = document.getElementById('val-fps');
const statsTbody = document.getElementById('stats-tbody');
let latestState = {};

function connect() {
    ws = new WebSocket(wsUrl);

    ws.onopen = () => {
        connStatus.textContent = "Connected";
        connStatus.classList.add('connected');
        logMsg("System: WebSocket Connected");
    };

    ws.onclose = () => {
        connStatus.textContent = "Disconnected - Reconnecting...";
        connStatus.classList.remove('connected');
        setTimeout(connect, 2000);
    };

    ws.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            if (data.type === "STATE") {
                latestState = data;
                updateUI(data);
                frameCount++;
            } else if (data.type === "STATISTICS") {
                updateStats(data.data);
            }
        } catch (e) {
            console.error("Invalid JSON", event.data);
        }
    };
}

// FPS calculation loop
function updateFPS() {
    const now = performance.now();
    const elapsed = now - lastTime;
    if (elapsed >= 1000) {
        const fps = Math.round((frameCount * 1000) / elapsed);
        if (valFps) valFps.textContent = fps;
        frameCount = 0;
        lastTime = now;
    }
    requestAnimationFrame(updateFPS);
}
requestAnimationFrame(updateFPS);

function updateStats(stats) {
    if (!statsTbody) return;
    statsTbody.innerHTML = '';
    
    // Convert object to array for sorting
    const statsArray = [];
    for (const [id, rate] of Object.entries(stats)) {
        // Priority is lower number = higher priority
        const priority = parseInt(id, 16);
        let value = "N/A";
        
        // Map ID to latest state value for display
        if (id === "0x100") value = latestState.steering;
        else if (id === "0x101") value = latestState.brake;
        else if (id === "0x102") value = latestState.throttle;
        else if (id === "0x200") value = latestState.speed;
        else if (id === "0x300") value = latestState.gear;
        else if (id === "0x400") value = latestState.climateTemp;

        statsArray.push({ id, priority, value, rate });
    }
    
    // Sort by priority (ascending)
    statsArray.sort((a, b) => a.priority - b.priority);
    
    statsArray.forEach(stat => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td><code>${stat.id}</code></td>
            <td>${stat.priority}</td>
            <td>${stat.value !== undefined ? stat.value : 'N/A'}</td>
            <td style="color: var(--accent-green); font-weight: bold;">${stat.rate}</td>
        `;
        statsTbody.appendChild(tr);
    });
}

function updateUI(state) {
    // Steering
    valSteering.textContent = state.steering;
    elSteeringWheel.style.transform = `rotate(${state.steering}deg)`;

    // Brake
    valBrake.textContent = state.brake;
    brakeBar.style.width = `${state.brake}%`;

    // Throttle
    valThrottle.textContent = state.throttle;
    throttleBar.style.width = `${state.throttle}%`;

    // HMI
    valSpeed.textContent = state.speed;
    valRpm.textContent = state.rpm;
    valGear.textContent = state.gear;
    if (state.climateTemp !== undefined) {
        valClimate.textContent = state.climateTemp;
    }
}

function logMsg(msg, isTx = false) {
    const div = document.createElement('div');
    div.className = `log-entry ${isTx ? 'tx' : ''}`;
    
    const time = new Date().toLocaleTimeString();
    div.textContent = `[${time}] ${msg}`;
    
    canLog.appendChild(div);
    canLog.scrollTop = canLog.scrollHeight;
}

btnSend.addEventListener('click', () => {
    const id = canIdInput.value.trim();
    const data = canDataInput.value.trim();

    if (!id || !data) return;

    if (ws && ws.readyState === WebSocket.OPEN) {
        const payload = { canId: id, data: data };
        ws.send(JSON.stringify(payload));
        logMsg(`TX: ID ${id} Data ${data}`, true);
    } else {
        logMsg("System: WebSocket not connected");
    }
});

btnReplayStart.addEventListener('click', () => {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ action: "replay_start" }));
        logMsg("System: Starting Replay");
    }
});

btnReplayStop.addEventListener('click', () => {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ action: "replay_stop" }));
        logMsg("System: Stopped Replay");
    }
});

// Allow Enter key to send
canDataInput.addEventListener('keypress', (e) => {
    if (e.key === 'Enter') btnSend.click();
});

// Initialize connection
connect();
