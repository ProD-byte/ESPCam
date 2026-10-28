#pragma once
#include <Arduino.h>

// Weboberfläche der Kamera (Port 80). Stream kommt von Port 81, Werte von
// /status, Einstellungen gehen an /control?var=..&val=..
static const char INDEX_HTML[] PROGMEM = R"rawliteral(<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0b0d12">
<title>ESPCam</title>
<style>
:root{--bg:#0b0d12;--card:#151922;--card2:#1d222d;--line:#262c39;--text:#e8ebf1;--muted:#8b93a5;--accent:#4f8cff;--accent2:#7b5cff;--ok:#2fd27a;--warn:#ffb020;--bad:#ff5468}
@media (prefers-color-scheme:light){:root{--bg:#f3f5f9;--card:#fff;--card2:#f0f2f7;--line:#e2e6ee;--text:#151922;--muted:#647089}}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
body{margin:0;min-height:100vh;background:var(--bg);color:var(--text);font:15px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif;-webkit-font-smoothing:antialiased}
svg{fill:none;stroke:currentColor;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
header{display:flex;align-items:center;gap:14px;padding:20px 16px 16px;max-width:1240px;margin:auto}
.logo{width:42px;height:42px;border-radius:12px;flex:none;background:linear-gradient(135deg,var(--accent),var(--accent2));display:grid;place-items:center;color:#fff;box-shadow:0 10px 30px -10px var(--accent)}
.logo svg{width:22px;height:22px}
h1{font-size:19px;margin:0;font-weight:700;letter-spacing:-.01em}
.sub{color:var(--muted);font-size:13px}
.chips{margin-left:auto;display:flex;gap:8px;min-width:0}
.chip{display:flex;align-items:center;gap:7px;background:var(--card);border:1px solid var(--line);border-radius:999px;padding:7px 12px;font-size:13px;color:var(--muted);white-space:nowrap;min-width:0}
.chip span{overflow:hidden;text-overflow:ellipsis}
.chip svg{width:15px;height:15px;flex:none}
.chip .dot{width:8px;height:8px;border-radius:50%;background:var(--ok);flex:none}
.chip.off .dot{background:var(--bad)}
main{display:grid;grid-template-columns:minmax(0,1fr) 340px;gap:16px;padding:0 16px 32px;max-width:1240px;margin:auto;align-items:start}
@media (max-width:920px){main{grid-template-columns:minmax(0,1fr)}}
@media (max-width:560px){.chip.ipc{display:none}header{padding-top:16px}}
.card{background:var(--card);border:1px solid var(--line);border-radius:20px;overflow:hidden;box-shadow:0 20px 40px -28px rgba(0,0,0,.5)}
.viewer{position:relative;aspect-ratio:4/3;background:#05060a;display:grid;place-items:center;overflow:hidden}
.viewer img{width:100%;height:100%;object-fit:contain;display:block}
.viewer:fullscreen{aspect-ratio:auto;width:100vw;height:100vh}
.viewer::after{content:"";position:absolute;inset:0;background:#fff;opacity:0;pointer-events:none}
.viewer.shot::after{animation:shot .45s ease-out}
@keyframes shot{from{opacity:.75}to{opacity:0}}
.badge{position:absolute;top:12px;display:flex;align-items:center;gap:7px;background:rgba(10,12,18,.6);backdrop-filter:blur(10px);-webkit-backdrop-filter:blur(10px);color:#fff;border-radius:999px;padding:5px 11px;font-size:12px;font-weight:650;letter-spacing:.04em}
.live{left:12px}
.live .dot{width:8px;height:8px;border-radius:50%;background:var(--bad);box-shadow:0 0 0 0 rgba(255,84,104,.7);animation:pulse 1.8s infinite}
.live.off .dot{background:#8b93a5;animation:none}
@keyframes pulse{70%{box-shadow:0 0 0 7px rgba(255,84,104,0)}100%{box-shadow:0 0 0 0 rgba(255,84,104,0)}}
.res{right:12px}
.ph{position:absolute;inset:0;display:grid;place-content:center;justify-items:center;gap:14px;color:#c9cfdb;font-size:14px;text-align:center;padding:20px}
.ring{width:38px;height:38px;border-radius:50%;border:3px solid rgba(255,255,255,.12);border-top-color:var(--accent);animation:spin .9s linear infinite}
.ph svg{width:40px;height:40px;opacity:.6}
@keyframes spin{to{transform:rotate(360deg)}}
.toolbar{display:flex;gap:10px;padding:14px;flex-wrap:wrap;justify-content:center}
.btn{display:inline-flex;align-items:center;justify-content:center;gap:8px;border:1px solid var(--line);background:var(--card2);color:var(--text);border-radius:13px;padding:11px 16px;font:inherit;font-weight:600;cursor:pointer;transition:transform .1s,border-color .15s,background .15s}
.btn:hover{border-color:var(--accent)}
.btn:active{transform:scale(.97)}
.btn svg{width:18px;height:18px;flex:none}
.btn.primary{background:linear-gradient(135deg,var(--accent),var(--accent2));border-color:transparent;color:#fff;min-width:130px}
.btn.on{background:var(--warn);border-color:transparent;color:#1a1300}
.btn.icon{padding:11px 13px}
aside{display:flex;flex-direction:column;gap:16px}
.section{padding:18px}
.section h2{display:flex;align-items:center;gap:8px;font-size:12px;text-transform:uppercase;letter-spacing:.08em;color:var(--muted);margin:0 0 6px;font-weight:650}
.section h2 svg{width:15px;height:15px}
.row{padding:12px 0}
.row label{display:flex;justify-content:space-between;align-items:baseline;font-weight:550;margin-bottom:10px}
.row output{color:var(--muted);font-weight:500;font-variant-numeric:tabular-nums}
select{width:100%;appearance:none;-webkit-appearance:none;font:inherit;padding:11px 40px 11px 13px;border-radius:12px;border:1px solid var(--line);background:var(--card2) url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 24 24' fill='none' stroke='%238b93a5' stroke-width='2.5' stroke-linecap='round' stroke-linejoin='round'%3E%3Cpolyline points='6 9 12 15 18 9'/%3E%3C/svg%3E") no-repeat right 12px center/16px;color:var(--text);cursor:pointer}
input[type=range]{-webkit-appearance:none;appearance:none;width:100%;height:6px;border-radius:999px;background:var(--line);outline:none;margin:6px 0}
input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;border-radius:50%;background:#fff;border:0;box-shadow:0 0 0 4px var(--accent),0 2px 6px rgba(0,0,0,.4);cursor:pointer}
input[type=range]::-moz-range-thumb{width:22px;height:22px;border-radius:50%;background:#fff;border:0;box-shadow:0 0 0 4px var(--accent),0 2px 6px rgba(0,0,0,.4);cursor:pointer}
.sw{display:flex;justify-content:space-between;align-items:center;padding:12px 0;font-weight:550;border-top:1px solid var(--line)}
.sw:first-of-type{margin-top:6px}
.switch{position:relative;width:46px;height:28px;flex:none}
.switch input{position:absolute;inset:0;opacity:0;margin:0;cursor:pointer;z-index:1}
.switch span{position:absolute;inset:0;border-radius:999px;background:var(--line);transition:background .2s}
.switch span::after{content:"";position:absolute;top:3px;left:3px;width:22px;height:22px;border-radius:50%;background:#fff;transition:transform .2s;box-shadow:0 1px 3px rgba(0,0,0,.35)}
.switch input:checked+span{background:var(--accent)}
.switch input:checked+span::after{transform:translateX(18px)}
.kv{display:grid;grid-template-columns:auto 1fr;gap:10px 12px;margin:10px 0 16px;font-size:14px}
.kv dt{color:var(--muted)}
.kv dd{margin:0;text-align:right;font-weight:550;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;font-variant-numeric:tabular-nums}
.actions{display:grid;grid-template-columns:1fr 1fr;gap:10px}
.toast{position:fixed;left:50%;bottom:24px;transform:translate(-50%,120px);background:var(--text);color:var(--bg);padding:12px 18px;border-radius:14px;font-weight:600;box-shadow:0 12px 30px rgba(0,0,0,.35);transition:transform .3s cubic-bezier(.2,.9,.3,1.2);z-index:9;max-width:calc(100% - 32px);text-align:center}
.toast.show{transform:translate(-50%,0)}
[hidden]{display:none!important}
</style>
</head>
<body>
<header>
  <div class="logo"><svg viewBox="0 0 24 24"><path d="M14.5 4h-5L7 7H4a2 2 0 0 0-2 2v9a2 2 0 0 0 2 2h16a2 2 0 0 0 2-2V9a2 2 0 0 0-2-2h-3l-2.5-3z"/><circle cx="12" cy="13" r="3"/></svg></div>
  <div><h1>ESPCam</h1><div class="sub" id="hostSub">&nbsp;</div></div>
  <div class="chips">
    <div class="chip" id="wifiChip"><span class="dot"></span><span id="wifiTxt">…</span></div>
    <div class="chip ipc"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="10"/><line x1="2" y1="12" x2="22" y2="12"/><path d="M12 2a15.3 15.3 0 0 1 4 10 15.3 15.3 0 0 1-4 10 15.3 15.3 0 0 1-4-10 15.3 15.3 0 0 1 4-10z"/></svg><span id="ipTxt">…</span></div>
  </div>
</header>

<main>
  <section class="card">
    <div class="viewer" id="viewer">
      <img id="stream" alt="">
      <div class="ph" id="ph"><div class="ring" id="phRing"></div><span id="phTxt">Verbinde mit Kamera …</span></div>
      <div class="badge live off" id="live"><span class="dot"></span><span id="liveTxt">LIVE</span></div>
      <div class="badge res" id="resBadge">–</div>
    </div>
    <div class="toolbar">
      <button class="btn primary" id="playBtn"></button>
      <button class="btn" id="snapBtn"><svg viewBox="0 0 24 24"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"/><polyline points="7 10 12 15 17 10"/><line x1="12" y1="15" x2="12" y2="3"/></svg>Foto</button>
      <button class="btn" id="flashBtn"><svg viewBox="0 0 24 24"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"/></svg>Licht</button>
      <button class="btn icon" id="fsBtn" aria-label="Vollbild"><svg viewBox="0 0 24 24"><path d="M8 3H5a2 2 0 0 0-2 2v3m18 0V5a2 2 0 0 0-2-2h-3m0 18h3a2 2 0 0 0 2-2v-3M3 16v3a2 2 0 0 0 2 2h3"/></svg></button>
    </div>
  </section>

  <aside>
    <section class="card section">
      <h2><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="3"/><path d="M19.4 15a1.65 1.65 0 0 0 .33 1.82l.06.06a2 2 0 1 1-2.83 2.83l-.06-.06a1.65 1.65 0 0 0-1.82-.33 1.65 1.65 0 0 0-1 1.51V21a2 2 0 1 1-4 0v-.09a1.65 1.65 0 0 0-1-1.51 1.65 1.65 0 0 0-1.82.33l-.06.06a2 2 0 1 1-2.83-2.83l.06-.06a1.65 1.65 0 0 0 .33-1.82 1.65 1.65 0 0 0-1.51-1H3a2 2 0 1 1 0-4h.09a1.65 1.65 0 0 0 1.51-1 1.65 1.65 0 0 0-.33-1.82l-.06-.06a2 2 0 1 1 2.83-2.83l.06.06a1.65 1.65 0 0 0 1.82.33h0a1.65 1.65 0 0 0 1-1.51V3a2 2 0 1 1 4 0v.09a1.65 1.65 0 0 0 1 1.51h0a1.65 1.65 0 0 0 1.82-.33l.06-.06a2 2 0 1 1 2.83 2.83l-.06.06a1.65 1.65 0 0 0-.33 1.82v0a1.65 1.65 0 0 0 1.51 1H21a2 2 0 1 1 0 4h-.09a1.65 1.65 0 0 0-1.51 1z"/></svg>Bild</h2>
      <div class="row">
        <label for="framesize">Auflösung</label>
        <select id="framesize">
          <option value="QVGA">QVGA · 320 × 240</option>
          <option value="VGA">VGA · 640 × 480</option>
          <option value="SVGA">SVGA · 800 × 600</option>
          <option value="XGA">XGA · 1024 × 768</option>
          <option value="HD">HD · 1280 × 720</option>
          <option value="SXGA">SXGA · 1280 × 1024</option>
          <option value="UXGA">UXGA · 1600 × 1200</option>
        </select>
      </div>
      <div class="row"><label for="quality">Bildqualität <output id="qualityOut"></output></label><input type="range" id="quality" min="0" max="30"></div>
      <div class="row"><label for="brightness">Helligkeit <output id="brightnessOut"></output></label><input type="range" id="brightness" min="-2" max="2"></div>
      <div class="row"><label for="contrast">Kontrast <output id="contrastOut"></output></label><input type="range" id="contrast" min="-2" max="2"></div>
      <div class="row"><label for="saturation">Sättigung <output id="saturationOut"></output></label><input type="range" id="saturation" min="-2" max="2"></div>
      <div class="sw">Spiegeln<label class="switch"><input type="checkbox" id="hmirror" aria-label="Spiegeln"><span></span></label></div>
      <div class="sw">Auf den Kopf drehen<label class="switch"><input type="checkbox" id="vflip" aria-label="Auf den Kopf drehen"><span></span></label></div>
    </section>

    <section class="card section">
      <h2><svg viewBox="0 0 24 24"><rect x="4" y="4" width="16" height="16" rx="2"/><rect x="9" y="9" width="6" height="6"/><line x1="9" y1="1" x2="9" y2="4"/><line x1="15" y1="1" x2="15" y2="4"/><line x1="9" y1="20" x2="9" y2="23"/><line x1="15" y1="20" x2="15" y2="23"/><line x1="20" y1="9" x2="23" y2="9"/><line x1="20" y1="14" x2="23" y2="14"/><line x1="1" y1="9" x2="4" y2="9"/><line x1="1" y1="14" x2="4" y2="14"/></svg>Gerät</h2>
      <dl class="kv">
        <dt>WLAN</dt><dd id="ssid">–</dd>
        <dt>Signal</dt><dd id="rssi">–</dd>
        <dt>IP-Adresse</dt><dd id="ip">–</dd>
        <dt>Laufzeit</dt><dd id="uptime">–</dd>
        <dt>Freier Speicher</dt><dd id="heap">–</dd>
      </dl>
      <div class="actions">
        <button class="btn" id="wifiBtn"><svg viewBox="0 0 24 24"><path d="M5 12.55a11 11 0 0 1 14.08 0"/><path d="M1.42 9a16 16 0 0 1 21.16 0"/><path d="M8.53 16.11a6 6 0 0 1 6.95 0"/><line x1="12" y1="20" x2="12.01" y2="20"/></svg>WLAN</button>
        <button class="btn" id="rebootBtn"><svg viewBox="0 0 24 24"><polyline points="23 4 23 10 17 10"/><path d="M20.49 15a9 9 0 1 1-2.12-9.36L23 10"/></svg>Neustart</button>
      </div>
    </section>
  </aside>
</main>
<div class="toast" id="toast"></div>

<script>
const $ = id => document.getElementById(id);
const STREAM = 'http://' + location.hostname + ':81/stream';
const BLANK = 'data:image/gif;base64,R0lGODlhAQABAAAAACw=';
const ICON_PAUSE = '<svg viewBox="0 0 24 24"><rect x="6" y="4" width="4" height="16" rx="1"/><rect x="14" y="4" width="4" height="16" rx="1"/></svg>Pause';
const ICON_PLAY = '<svg viewBox="0 0 24 24"><polygon points="6 3 20 12 6 21 6 3"/></svg>Abspielen';
const img = $('stream');
let streaming = false, retry, watcher, toastTimer, info = {};

function toast(msg) {
  const t = $('toast');
  t.textContent = msg;
  t.classList.add('show');
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => t.classList.remove('show'), 3000);
}
function placeholder(text, spinning) {
  $('ph').hidden = !text;
  $('phTxt').textContent = text || '';
  $('phRing').hidden = !spinning;
  img.style.visibility = text ? 'hidden' : '';  // kein "kaputtes Bild"-Symbol
}
function setLive(on) {
  $('live').classList.toggle('off', !on);
  $('liveTxt').textContent = on ? 'LIVE' : streaming ? 'VERBINDE' : 'PAUSE';
}

// --- Stream ---
function play() {
  streaming = true;
  clearTimeout(retry);
  placeholder('Verbinde mit Kamera …', true);
  setLive(false);
  $('playBtn').innerHTML = ICON_PAUSE;
  img.src = STREAM + '?t=' + Date.now();
  clearInterval(watcher);
  watcher = setInterval(() => {
    if (streaming && img.naturalWidth > 1) { placeholder(null); setLive(true); clearInterval(watcher); }
  }, 250);
}
function pause() {
  streaming = false;
  clearTimeout(retry);
  clearInterval(watcher);
  img.src = BLANK;
  placeholder('Stream pausiert', false);
  setLive(false);
  $('playBtn').innerHTML = ICON_PLAY;
}
img.onerror = () => {
  if (!streaming) return;
  setLive(false);
  placeholder('Verbindung unterbrochen – neuer Versuch …', true);
  clearTimeout(retry);
  retry = setTimeout(play, 2000);
};
$('playBtn').onclick = () => streaming ? pause() : play();

// --- Foto, Licht, Vollbild ---
$('snapBtn').onclick = async () => {
  try {
    const r = await fetch('/capture?t=' + Date.now());
    if (!r.ok) throw 0;
    const blob = await r.blob();
    const d = new Date(), p = n => String(n).padStart(2, '0');
    const a = document.createElement('a');
    a.href = URL.createObjectURL(blob);
    a.download = `espcam-${d.getFullYear()}${p(d.getMonth() + 1)}${p(d.getDate())}-${p(d.getHours())}${p(d.getMinutes())}${p(d.getSeconds())}.jpg`;
    document.body.appendChild(a);
    a.click();
    a.remove();
    setTimeout(() => URL.revokeObjectURL(a.href), 5000);
    const v = $('viewer');
    v.classList.remove('shot'); void v.offsetWidth; v.classList.add('shot');
    toast('Foto gespeichert');
  } catch (e) { toast('Foto fehlgeschlagen'); }
};
function showFlash(on) { $('flashBtn').classList.toggle('on', on); }
$('flashBtn').onclick = async () => {
  const on = !$('flashBtn').classList.contains('on');
  showFlash(on);
  try { await fetch('/flash?on=' + (on ? 1 : 0)); } catch (e) { showFlash(!on); toast('Licht nicht erreichbar'); }
};
$('fsBtn').onclick = () => {
  const v = $('viewer');
  if (document.fullscreenElement || document.webkitFullscreenElement) {
    (document.exitFullscreen || document.webkitExitFullscreen).call(document);
  } else {
    const req = v.requestFullscreen || v.webkitRequestFullscreen;
    if (req) req.call(v); else toast('Vollbild wird hier nicht unterstützt');
  }
};

// --- Bildeinstellungen ---
function control(name, val) {
  return fetch(`/control?var=${name}&val=${encodeURIComponent(val)}`)
    .then(r => { if (!r.ok) throw 0; })
    .catch(() => toast('Einstellung fehlgeschlagen'));
}
const signed = v => (v > 0 ? '+' : '') + v;
const qualityText = v => v >= 22 ? 'Hoch' : v >= 12 ? 'Mittel' : 'Niedrig';
$('framesize').onchange = e => { $('resBadge').textContent = e.target.value; control('framesize', e.target.value); };
$('quality').oninput = e => $('qualityOut').textContent = qualityText(+e.target.value);
$('quality').onchange = e => control('quality', 40 - e.target.value);  // Sensor: 10 = beste Qualität
for (const id of ['brightness', 'contrast', 'saturation']) {
  $(id).oninput = e => $(id + 'Out').textContent = signed(+e.target.value);
  $(id).onchange = e => control(id, e.target.value);
}
for (const id of ['hmirror', 'vflip']) $(id).onchange = e => control(id, e.target.checked ? 1 : 0);

function applySettings(s) {
  $('framesize').value = s.framesize;
  $('resBadge').textContent = s.framesize;
  $('quality').value = 40 - s.quality;
  $('qualityOut').textContent = qualityText(40 - s.quality);
  for (const id of ['brightness', 'contrast', 'saturation']) {
    $(id).value = s[id];
    $(id + 'Out').textContent = signed(s[id]);
  }
  $('hmirror').checked = !!s.hmirror;
  $('vflip').checked = !!s.vflip;
}

// --- Gerätestatus ---
const signalText = r => r > -55 ? 'sehr gut' : r > -65 ? 'gut' : r > -75 ? 'mäßig' : 'schwach';
function uptimeText(sec) {
  const d = Math.floor(sec / 86400), h = Math.floor(sec % 86400 / 3600), m = Math.floor(sec % 3600 / 60);
  return d ? `${d} T ${h} Std` : h ? `${h} Std ${m} Min` : `${m} Min`;
}
async function status(first) {
  try {
    const r = await fetch('/status', {cache: 'no-store'});
    const s = await r.json();
    info = s;
    $('wifiChip').classList.remove('off');
    $('wifiTxt').textContent = s.ssid;
    $('ipTxt').textContent = s.ip;
    $('hostSub').textContent = s.host + '.local';
    $('ssid').textContent = s.ssid;
    $('rssi').textContent = `${signalText(s.rssi)} · ${s.rssi} dBm`;
    $('ip').textContent = s.ip;
    $('uptime').textContent = uptimeText(s.uptime);
    $('heap').textContent = Math.round(s.heap / 1024) + ' kB' + (s.psram ? ` · PSRAM ${Math.round(s.psram / 1024)} kB` : '');
    showFlash(s.flash);
    if (first) applySettings(s);
  } catch (e) {
    $('wifiChip').classList.add('off');
    $('wifiTxt').textContent = 'offline';
  }
}

$('wifiBtn').onclick = () => {
  const ap = info.setupAp || 'ESPCam-Setup';
  if (!confirm(`Neues WLAN hinzufügen?\n\nDie Kamera startet neu und öffnet das WLAN „${ap}“. Verbinde dich damit am Handy. Gespeicherte WLANs bleiben erhalten.`)) return;
  pause();
  fetch('/wifi/setup').catch(() => {});
  toast(`Mit WLAN „${ap}“ verbinden`);
};
$('rebootBtn').onclick = () => {
  if (!confirm('Kamera jetzt neu starten?')) return;
  pause();
  fetch('/restart').catch(() => {});
  toast('Kamera startet neu …');
  setTimeout(() => location.reload(), 9000);
};

status(true);
setInterval(status, 5000);
play();
</script>
</body>
</html>)rawliteral";
