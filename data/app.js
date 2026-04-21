(() => {
  const statusDot  = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');
  const demoBadge  = document.getElementById('demoBadge');
  const gestureName = document.getElementById('gestureName');
  const gestureIdEl = document.getElementById('gestureId');
  const rollFill = document.getElementById('rollFill');
  const pitchFill = document.getElementById('pitchFill');
  const flex1Fill = document.getElementById('flex1Fill');
  const flex2Fill = document.getElementById('flex2Fill');
  const rollVal = document.getElementById('rollVal');
  const pitchVal = document.getElementById('pitchVal');
  const flex1Val = document.getElementById('flex1Val');
  const flex2Val = document.getElementById('flex2Val');
  const servoPanEl = document.getElementById('servoPan');
  const servoTiltEl = document.getElementById('servoTilt');
  const logBody = document.getElementById('logBody');

  // ===== Three.js scene =====
  const container = document.getElementById('threeContainer');
  const scene = new THREE.Scene();
  scene.background = new THREE.Color(0x161b22);

  const camera = new THREE.PerspectiveCamera(45, 1, 0.1, 100);
  camera.position.set(3, 2.2, 4);
  camera.lookAt(0, 0, 0);

  const renderer = new THREE.WebGLRenderer({ antialias: true });
  renderer.setPixelRatio(window.devicePixelRatio || 1);
  container.appendChild(renderer.domElement);

  function resize() {
    const w = container.clientWidth;
    const h = container.clientHeight;
    renderer.setSize(w, h, false);
    camera.aspect = w / h;
    camera.updateProjectionMatrix();
  }
  window.addEventListener('resize', resize);

  // Lights
  scene.add(new THREE.AmbientLight(0xffffff, 0.35));
  const dir = new THREE.DirectionalLight(0xffffff, 0.9);
  dir.position.set(5, 6, 4);
  scene.add(dir);

  // Hand "box" — different colored faces to show orientation
  const faceColors = [0x2e7d32, 0x4ade80, 0x22d3ee, 0x60a5fa, 0xf87171, 0xfbbf24];
  const mats = faceColors.map(c => new THREE.MeshStandardMaterial({
    color: c, roughness: 0.4, metalness: 0.15
  }));
  const geo = new THREE.BoxGeometry(1.6, 0.35, 1.05);
  const hand = new THREE.Mesh(geo, mats);
  scene.add(hand);

  // Wrist marker
  const wrist = new THREE.Mesh(
    new THREE.CylinderGeometry(0.08, 0.08, 0.6, 16),
    new THREE.MeshStandardMaterial({ color: 0xe6edf3, roughness: 0.6 })
  );
  wrist.rotation.z = Math.PI / 2;
  wrist.position.x = -1.1;
  hand.add(wrist);

  // Floor grid
  const grid = new THREE.GridHelper(6, 12, 0x30363d, 0x1f2630);
  grid.position.y = -1.0;
  scene.add(grid);

  let targetRoll = 0, targetPitch = 0;

  function animate() {
    requestAnimationFrame(animate);
    hand.rotation.x += ((THREE.MathUtils.degToRad(-targetPitch)) - hand.rotation.x) * 0.15;
    hand.rotation.z += ((THREE.MathUtils.degToRad(targetRoll)) - hand.rotation.z) * 0.15;
    renderer.render(scene, camera);
  }
  resize();
  animate();

  // ===== Telemetry rendering =====
  function setCenteredFill(el, value, minV, maxV) {
    const clamped = Math.max(minV, Math.min(maxV, value));
    const pct = clamped / maxV;
    if (pct >= 0) {
      el.style.left = '50%';
      el.style.width = (pct * 50) + '%';
    } else {
      el.style.left = (50 + pct * 50) + '%';
      el.style.width = (-pct * 50) + '%';
    }
  }

  function updateTelemetry(d) {
    rollVal.textContent  = d.roll.toFixed(1) + '°';
    pitchVal.textContent = d.pitch.toFixed(1) + '°';
    flex1Val.textContent = d.flex1.toFixed(0) + '%';
    flex2Val.textContent = d.flex2.toFixed(0) + '%';
    setCenteredFill(rollFill, d.roll, -90, 90);
    setCenteredFill(pitchFill, d.pitch, -90, 90);
    flex1Fill.style.width = Math.max(0, Math.min(100, d.flex1)) + '%';
    flex2Fill.style.width = Math.max(0, Math.min(100, d.flex2)) + '%';
    servoPanEl.textContent = d.servoPan;
    servoTiltEl.textContent = d.servoTilt;
  }

  // ===== Gesture log (only on change) =====
  let lastGesture = null;
  const MAX_LOG = 50;

  function addLogRow(d) {
    const tr = document.createElement('tr');
    tr.className = 'g-' + d.gesture;
    const time = new Date().toLocaleTimeString();
    tr.innerHTML = `
      <td>${time}</td>
      <td>${d.gesture}</td>
      <td>${d.roll.toFixed(1)}</td>
      <td>${d.pitch.toFixed(1)}</td>
      <td>${d.flex1.toFixed(0)}</td>
      <td>${d.flex2.toFixed(0)}</td>
    `;
    logBody.prepend(tr);
    while (logBody.children.length > MAX_LOG) {
      logBody.removeChild(logBody.lastChild);
    }
  }

  function handleMessage(d) {
    targetRoll = d.roll;
    targetPitch = d.pitch;
    updateTelemetry(d);
    if (d.gesture !== lastGesture) {
      lastGesture = d.gesture;
      gestureName.textContent = d.gesture;
      gestureIdEl.textContent = d.gestureId;
      if (d.gesture !== 'NONE') addLogRow(d);
    }
  }

  // ===== Connection state =====
  let ws = null;
  let demoMode = false;
  let demoTimer = null;

  function setConnected(isConn) {
    statusDot.classList.toggle('connected', isConn);
    statusDot.classList.toggle('disconnected', !isConn);
    statusText.textContent = isConn ? 'Connected' : 'Disconnected';
  }

  function setDemoMode(on) {
    demoMode = on;
    demoBadge.classList.toggle('hidden', !on);
    if (on) {
      statusText.textContent = 'Simulated data';
      startDemoLoop();
    } else {
      stopDemoLoop();
    }
  }

  // ===== Demo data generator =====
  // Cycles through every gesture so the dashboard demonstrates itself.
  // roll/pitch follow smooth sinusoids; flex values are scripted per gesture.
  const DEMO_SCRIPT = [
    { gesture: 'NONE',       gestureId: 0, durationMs: 1800, flex1: 10, flex2: 10, rollOffset: 0,   pitchOffset: 0 },
    { gesture: 'TILT_LEFT',  gestureId: 1, durationMs: 1800, flex1: 15, flex2: 15, rollOffset: -55, pitchOffset: 0 },
    { gesture: 'NONE',       gestureId: 0, durationMs: 900,  flex1: 10, flex2: 10, rollOffset: 0,   pitchOffset: 0 },
    { gesture: 'TILT_RIGHT', gestureId: 2, durationMs: 1800, flex1: 15, flex2: 15, rollOffset: 55,  pitchOffset: 0 },
    { gesture: 'NONE',       gestureId: 0, durationMs: 900,  flex1: 10, flex2: 10, rollOffset: 0,   pitchOffset: 0 },
    { gesture: 'TILT_UP',    gestureId: 3, durationMs: 1800, flex1: 15, flex2: 15, rollOffset: 0,   pitchOffset: -55 },
    { gesture: 'NONE',       gestureId: 0, durationMs: 900,  flex1: 10, flex2: 10, rollOffset: 0,   pitchOffset: 0 },
    { gesture: 'TILT_DOWN',  gestureId: 4, durationMs: 1800, flex1: 15, flex2: 15, rollOffset: 0,   pitchOffset: 55 },
    { gesture: 'NONE',       gestureId: 0, durationMs: 900,  flex1: 10, flex2: 10, rollOffset: 0,   pitchOffset: 0 },
    { gesture: 'GRAB',       gestureId: 5, durationMs: 2000, flex1: 88, flex2: 92, rollOffset: 0,   pitchOffset: 0 },
    { gesture: 'NONE',       gestureId: 0, durationMs: 900,  flex1: 10, flex2: 10, rollOffset: 0,   pitchOffset: 0 },
    { gesture: 'POINT',      gestureId: 6, durationMs: 2000, flex1: 18, flex2: 78, rollOffset: 0,   pitchOffset: 0 },
    { gesture: 'NONE',       gestureId: 0, durationMs: 1200, flex1: 10, flex2: 10, rollOffset: 0,   pitchOffset: 0 },
  ];

  let demoStart = 0;
  let demoIndex = 0;
  let demoStepStart = 0;

  function lerp(a, b, t) { return a + (b - a) * t; }
  function easeInOut(t) { return t < 0.5 ? 2*t*t : 1 - Math.pow(-2*t + 2, 2)/2; }

  function startDemoLoop() {
    if (demoTimer) return;
    demoStart = performance.now();
    demoStepStart = demoStart;
    demoIndex = 0;
    // 50 Hz tick — matches the firmware's broadcast cadence closely enough.
    demoTimer = setInterval(demoTick, 20);
  }

  function stopDemoLoop() {
    if (demoTimer) { clearInterval(demoTimer); demoTimer = null; }
  }

  function demoTick() {
    const now = performance.now();
    const step = DEMO_SCRIPT[demoIndex];
    const elapsed = now - demoStepStart;

    if (elapsed >= step.durationMs) {
      demoIndex = (demoIndex + 1) % DEMO_SCRIPT.length;
      demoStepStart = now;
    }

    const t = Math.min(1, elapsed / step.durationMs);
    const ramp = easeInOut(Math.min(1, t * 2));
    const fade = easeInOut(Math.min(1, (1 - t) * 2));
    const env  = Math.min(ramp, fade);

    const wobble = Math.sin(now / 280) * 4;
    const roll  = step.rollOffset  * env + wobble * 0.5;
    const pitch = step.pitchOffset * env + Math.cos(now / 320) * 3;

    const data = {
      gesture:   step.gesture,
      gestureId: step.gestureId,
      roll, pitch,
      flex1: step.flex1 + Math.sin(now / 220) * 1.2,
      flex2: step.flex2 + Math.cos(now / 240) * 1.2,
      servoPan:  Math.round(90 + roll  * 0.6),
      servoTilt: Math.round(90 + pitch * 0.6),
    };
    handleMessage(data);
  }

  // ===== WebSocket with auto-reconnect + demo fallback =====
  // If we're opened from file:// (no ESP32 reachable), skip WS entirely and go
  // straight to demo mode. Otherwise try to connect; if the first attempt
  // doesn't open within DEMO_FALLBACK_MS, flip into demo mode but keep
  // retrying in the background so a real device can take over later.
  const DEMO_FALLBACK_MS = 2500;
  let firstAttempt = true;

  function connect() {
    if (location.protocol === 'file:') {
      setConnected(false);
      setDemoMode(true);
      return;
    }

    const host = window.location.hostname || '192.168.4.1';
    let opened = false;

    try {
      ws = new WebSocket('ws://' + host + ':81');
    } catch (e) {
      scheduleDemoFallback();
      setTimeout(connect, 2000);
      return;
    }

    ws.onopen = () => {
      opened = true;
      setDemoMode(false);
      setConnected(true);
    };
    ws.onclose = () => {
      setConnected(false);
      if (firstAttempt && !opened) scheduleDemoFallback();
      setTimeout(connect, 2000);
    };
    ws.onerror = () => { try { ws.close(); } catch (_) {} };
    ws.onmessage = (ev) => {
      try {
        const data = JSON.parse(ev.data);
        if (demoMode) setDemoMode(false);
        setConnected(true);
        handleMessage(data);
      } catch (e) { /* ignore malformed frames */ }
    };

    if (firstAttempt) {
      firstAttempt = false;
      setTimeout(() => {
        if (!opened && !demoMode) setDemoMode(true);
      }, DEMO_FALLBACK_MS);
    }
  }

  function scheduleDemoFallback() {
    if (!demoMode) setDemoMode(true);
  }

  connect();
})();
