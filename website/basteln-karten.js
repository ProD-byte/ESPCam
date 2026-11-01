// Raeumt daveslab.de/basteln auf: oben eine Uebersicht mit Projekt-Karten, darunter ist immer nur
// EIN Projekt aufgeklappt (Flasher + Anleitung). Links wie daveslab.de/basteln/#wetter oeffnen
// direkt das passende Projekt. Gleiches Prinzip wie die anderen basteln-*.js: jede Ersetzung muss
// genau einmal passen, sonst wird nichts gespeichert.
//
// SO BENUTZEN (selbst, in Chrome):
//   1. Bei https://daveslab.de/wp-admin/ anmelden
//   2. F12 -> Reiter "Console" -> diese ganze Datei hineinkopieren -> Enter
//   3. Rueckfrage "Speichern?" -> OK. Danach https://daveslab.de/basteln/ neu laden.
//   WordPress hebt die alte Fassung als Revision auf (Seite bearbeiten -> Revisionen).
//
// NEUES PROJEKT SPAETER: Karte in KARTEN unten ergaenzen und in PROJ (im Skript KARTEN_SKRIPT)
// eintragen, welche <section id="..."> dazugehoeren.
function kartenEinbauen(raw) {
  const fehler = [];
  let out = raw;
  function ersetze(name, such, neu) {
    const treffer = typeof such === 'string' ? out.split(such).length - 1 : (out.match(new RegExp(such.source, 'g')) || []).length;
    if (treffer !== 1) { fehler.push(name + ': ' + treffer + ' Treffer'); return; }
    out = out.replace(such, typeof neu === 'function' ? neu : () => neu);
  }
  if (out.includes('class="dl-projekte')) return {out: raw, fehler: ['Karten sind schon eingebaut']};

  // --- Knopfreihe im Kopfbereich -> Projekt-Karten ---
  ersetze('hero knoepfe', /<p class="rv" style="display:flex;gap:10px;flex-wrap:wrap;justify-content:center;margin-top:22px">[\s\S]*?<\/p>/, KARTEN);

  // --- Kopfzeile ueber der Ueberschrift ---
  ersetze('eyebrow de', 'data-t="eyebrow"><b>BASTELN</b>Roboauto · ESPCam', 'data-t="eyebrow"><b>BASTELN</b>5 Projekte · ESP32');
  ersetze('eyebrow en', "<b>MAKING</b>Roboauto · ESPCam'", "<b>MAKING</b>5 projects · ESP32'");
  ersetze('eyebrow es', "<b>BRICOLAJE</b>Roboauto · ESPCam'", "<b>BRICOLAJE</b>5 proyectos · ESP32'");
  ersetze('eyebrow fr', "<b>BRICOLAGE</b>Roboauto · ESPCam'", "<b>BRICOLAGE</b>5 projets · ESP32'");

  // --- Stil ---
  ersetze('stil', '.dl-lang{gap:2px}}', '.dl-lang{gap:2px}}\n' + KARTEN_STIL);

  // --- Aufklappen: ganz vorn im Seiten-Skript (vor dem Einsammeln der deutschen Texte) ---
  ersetze('skript', "root.classList.add('js');", "root.classList.add('js');\n" + KARTEN_SKRIPT);

  // --- Uebersetzungen ---
  ersetze('en', "en:{nav_spiele:'Games',", "en:{nav_spiele:'Games'," + KARTEN_EN + ',');
  ersetze('es', "es:{nav_spiele:'Juegos',", "es:{nav_spiele:'Juegos'," + KARTEN_ES + ',');
  ersetze('fr', "fr:{nav_spiele:'Jeux',", "fr:{nav_spiele:'Jeux'," + KARTEN_FR + ',');

  return {out: fehler.length ? raw : out, fehler};
}

function karte(id, p, ic, titel, text, board) {
  return `<a class="dl-karte" href="#${id}" data-p="${p}" aria-expanded="false"><span class="ic">${ic}</span><h3>${titel}</h3>` +
         `<p data-t="k_${p}">${text}</p><small>${board}</small><span class="auf-zu" data-t="k_auf">Öffnen</span></a>`;
}

var KARTEN = `<div class="dl-projekte rv" id="projekte">
        ${karte('flasher', 'robo', '🚗', 'Roboauto', 'Roboterauto mit Panzerlenkung – fahren per App oder im Browser.', 'ESP32-S3 Super Mini')}
        ${karte('xiao', 'xiao', '🎥', 'Roboauto XIAO', 'Das Roboauto mit Live-Kamera, Mikrofon und SD-Karte.', 'XIAO ESP32S3 Sense')}
        ${karte('app', 'app', '📱', 'Roboauto-App', 'Fahren per Bluetooth oder WLAN – für Android.', 'Android 8+')}
        ${karte('espcam', 'cam', '📷', 'ESPCam', 'WLAN-Kamera mit Livebild im Browser.', 'AI-Thinker ESP32-CAM')}
        ${karte('wetter', 'wetter', '🌦️', 'Wetterstation', 'Animiertes Wetter für deinen Ort auf dem Mini-Display.', 'LILYGO T-Display')}
      </div>`;

var KARTEN_STIL = `/* ---- Projekt-Karten: Uebersicht oben, immer nur ein Projekt aufgeklappt ---- */
.dl-sec{scroll-margin-top:84px}
.dl.js .dl-sec.zu{display:none}
.dl-projekte{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,200px),1fr));gap:14px;max-width:1120px;margin:40px auto 0;text-align:left;scroll-margin-top:96px}
.dl a.dl-karte{position:relative;display:flex;flex-direction:column;gap:6px;padding:20px 18px 16px;border-radius:20px;border:1px solid var(--line)!important;background:linear-gradient(180deg,rgba(255,255,255,.05),rgba(255,255,255,.012)),var(--card);transition:transform .25s,border-color .25s,box-shadow .25s;cursor:pointer}
.dl a.dl-karte:hover{transform:translateY(-4px);border-color:rgba(139,92,246,.6)!important;box-shadow:0 22px 50px -26px rgba(139,92,246,.8)}
.dl a.dl-karte.auf{border-color:rgba(34,211,238,.75)!important;box-shadow:0 0 0 1px rgba(34,211,238,.45),0 22px 50px -26px rgba(34,211,238,.7)}
.dl-karte .ic{width:48px;height:48px;border-radius:14px;display:grid;place-items:center;font-size:26px;line-height:1;margin-bottom:6px;background:linear-gradient(135deg,rgba(244,114,182,.22),rgba(139,92,246,.22));border:1px solid var(--line)}
.dl-karte.auf .ic{background:linear-gradient(135deg,rgba(34,211,238,.25),rgba(139,92,246,.25))}
.dl-karte h3{margin:0;font-size:18px;line-height:1.25}
.dl-hero .dl-karte p{max-width:none;margin:0;font-size:14px;line-height:1.45;color:var(--muted);flex:1}
.dl-karte small{color:#7dd3fc;font-size:12px;font-weight:700;letter-spacing:.03em}
.dl-karte .auf-zu{display:flex;align-items:center;gap:8px;margin-top:8px;font-size:13px;font-weight:800;color:#fff}
.dl-karte .auf-zu:after{content:"";width:7px;height:7px;border-right:2px solid currentColor;border-bottom:2px solid currentColor;transform:translateY(-2px) rotate(45deg);transition:transform .25s}
.dl-karte.auf .auf-zu:after{transform:translateY(2px) rotate(-135deg)}
.dl a.dl-zurueck{display:inline-flex;align-items:center;gap:6px;margin:0 0 22px;padding:8px 16px;border-radius:999px;border:1px solid var(--line)!important;font-size:14px;font-weight:700;color:var(--muted);background:rgba(255,255,255,.04)}
.dl a.dl-zurueck:hover{color:#fff;border-color:rgba(255,255,255,.25)!important}`;

var KARTEN_SKRIPT = `/* ---- Projekt-Karten: immer nur ein Projekt aufgeklappt, #anker oeffnet das passende ---- */
var PROJ={robo:['flasher','anleitung'],xiao:['xiao'],app:['app'],cam:['espcam'],wetter:['wetter']};
var zuP={};Object.keys(PROJ).forEach(function(p){PROJ[p].forEach(function(id){zuP[id]=p;});});
function projSecs(p){return PROJ[p].map(function(id){return document.getElementById(id);}).filter(Boolean);}
Object.keys(PROJ).forEach(function(p){var s=projSecs(p)[0];if(!s)return;var w=s.querySelector('.dl-wrap');var a=document.createElement('a');a.className='dl-zurueck';a.href='#projekte';a.setAttribute('data-t','zurueck');a.textContent='← Alle Projekte';a.addEventListener('click',function(e){e.preventDefault();oeffne(null,true);});w.insertBefore(a,w.firstChild);});
function oeffne(p,rollen){Object.keys(PROJ).forEach(function(q){projSecs(q).forEach(function(s){s.classList.toggle('zu',q!==p);if(q===p)s.querySelectorAll('.rv').forEach(function(el){el.classList.add('in');});});});
  root.querySelectorAll('.dl-karte').forEach(function(k){var an=k.getAttribute('data-p')===p;k.classList.toggle('auf',an);k.setAttribute('aria-expanded',an?'true':'false');});
  try{history.replaceState(null,'',p?'#'+PROJ[p][0]:location.pathname+location.search);}catch(e){}
  if(rollen){var z=p?projSecs(p)[0]:document.getElementById('projekte');if(z)setTimeout(function(){z.scrollIntoView({behavior:'smooth',block:'start'});},40);}}
root.querySelectorAll('.dl-karte').forEach(function(k){k.addEventListener('click',function(e){e.preventDefault();var p=k.getAttribute('data-p');oeffne(k.classList.contains('auf')?null:p,true);});});
function ausAnker(){var p=zuP[location.hash.slice(1)];oeffne(p||null,!!p);}
ausAnker();addEventListener('hashchange',ausAnker);`;

var KARTEN_EN = "k_robo:'Robot car with tank steering – drive it with the app or in the browser.',k_xiao:'The Roboauto with live camera, microphone and SD card.',k_app:'Drive via Bluetooth or Wi-Fi – for Android.',k_cam:'Wi-Fi camera with live view in the browser.',k_wetter:'Animated weather for your town on a mini display.',k_auf:'Open',zurueck:'← All projects'";
var KARTEN_ES = "k_robo:'Coche robot con dirección de oruga: se maneja con la app o en el navegador.',k_xiao:'El Roboauto con cámara en directo, micrófono y tarjeta SD.',k_app:'Conduce por Bluetooth o Wi-Fi, para Android.',k_cam:'Cámara Wi-Fi con imagen en directo en el navegador.',k_wetter:'El tiempo animado de tu ciudad en una mini pantalla.',k_auf:'Abrir',zurueck:'← Todos los proyectos'";
var KARTEN_FR = "k_robo:'Voiture robot à direction de char – pilotée avec l’app ou dans le navigateur.',k_xiao:'Le Roboauto avec caméra en direct, micro et carte SD.',k_app:'Pilote en Bluetooth ou en Wi-Fi – pour Android.',k_cam:'Caméra Wi-Fi avec image en direct dans le navigateur.',k_wetter:'La météo animée de ta ville sur un mini écran.',k_auf:'Ouvrir',zurueck:'← Tous les projets'";

// ---------- Ausfuehren: Seite holen, umbauen, nach Rueckfrage speichern ----------
(async function kartenSpeichern() {
  if (!location.hostname.endsWith('daveslab.de')) return;  // (beim Testen woanders nichts tun)
  const nonce = (typeof wpApiSettings !== 'undefined' && wpApiSettings.nonce) ||
                await (await fetch('/wp-admin/admin-ajax.php?action=rest-nonce', {credentials: 'include'})).text();
  if (!nonce || nonce === '0') { alert('Nicht angemeldet - bitte erst bei WordPress anmelden.'); return; }
  const kopf = {'X-WP-Nonce': nonce, 'Content-Type': 'application/json'};
  const liste = await (await fetch('/wp-json/wp/v2/pages?slug=basteln&context=edit&_fields=id,content', {headers: kopf, credentials: 'include'})).json();
  if (!Array.isArray(liste) || !liste[0]) { alert('Seite "basteln" nicht gefunden'); return; }
  const seite = liste[0];
  const {out, fehler} = kartenEinbauen(seite.content.raw);
  if (fehler.length) { alert('Nichts geaendert:\n' + fehler.join('\n')); return; }
  if (!confirm('Basteln-Seite auf Projekt-Karten umbauen und speichern?\n(' + seite.content.raw.length + ' -> ' + out.length + ' Zeichen)')) return;
  const r = await fetch('/wp-json/wp/v2/pages/' + seite.id, {method: 'POST', headers: kopf, credentials: 'include', body: JSON.stringify({content: out})});
  alert(r.ok ? 'Gespeichert! Jetzt https://daveslab.de/basteln/ neu laden.' : 'Fehler beim Speichern: ' + r.status);
})();
