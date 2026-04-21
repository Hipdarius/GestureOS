(() => {
  const statusDot  = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');
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
    // Smooth interpolation toward target
    hand.rotation.x += ((THREE.MathUtils.degToRad(-targetPitch)) - hand.rotation.x) * 0.15;
    hand.rotation.z += ((THREE.MathUtils.degToRad(targetRoll)) - hand.rotation.z) * 0.15;
    renderer.render(scene, camera);
  }
  resize();
  animate();

  // ===== Telemetry rendering =====
  function setCenteredFill(el, value, minV, maxV) {
    const clamped = Math.max(minV, Math.min(maxV, value));
    const pct = clamped / maxV; // -1..1 when min=-max
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

  // ===== WebSocket with auto-reconnect =====
  let ws = null;
  function setConnected(isConn) {
    statusDot.classList.toggle('connected', isConn);
    statusDot.classList.toggle('disconnected', !isConn);
    statusText.textContent = isConn ? 'Connected' : 'Disconnected';
  }

  function connect() {
    const host = window.location.hostname || '192.168.4.1';
    ws = new WebSocket('ws://' + host + ':81');
    ws.onopen    = () => setConnected(true);
    ws.onclose   = () => { setConnected(false); setTimeout(connect, 2000); };
    ws.onerror   = () => { try { ws.close(); } catch (_) {} };
    ws.onmessage = (ev) => {
      try {
        const data = JSON.parse(ev.data);
        handleMessage(data);
      } catch (e) {
        // ignore malformed frames
      }
    };
  }
  connect();
})();
