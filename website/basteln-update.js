// Baut ESPCam in den Rohinhalt von daveslab.de/basteln ein.
// Jede Ersetzung muss genau einmal passen, sonst wird nichts gespeichert.
function espcamEinbauen(raw) {
  const fehler = [];
  let out = raw;
  function ersetze(name, such, neu) {
    const treffer = typeof such === 'string' ? out.split(such).length - 1 : (out.match(new RegExp(such.source, 'g')) || []).length;
    if (treffer !== 1) { fehler.push(name + ': ' + treffer + ' Treffer'); return; }
    out = out.replace(such, neu);
  }
  if (out.includes('id="espcam"')) return {out: raw, fehler: ['ESPCam ist schon eingebaut']};

  // --- Kopfbereich: gilt jetzt für beide Projekte ---
  ersetze('eyebrow', /(data-t="eyebrow"><b>BASTELN<\/b>)Roboauto · ESP32-S3/, '$1Roboauto · ESPCam');
  ersetze('h1', 'Roboauto <span class="dl-grad">Flasher</span></h1>', 'DavesLab <span class="dl-grad">Flasher</span></h1>');
  ersetze('hero_p', /(data-t="hero_p">)[^<]*(<\/p>)/,
    '$1Firmware für meine Bastelprojekte direkt aus dem Browser aufspielen – ohne Programmieren, ohne Installation.$2\n' +
    '      <p class="rv" style="display:flex;gap:10px;flex-wrap:wrap;justify-content:center;margin-top:22px"><a class="dl-btn" href="#flasher">🚗 Roboauto</a><a class="dl-btn" href="#espcam">📷 ESPCam</a></p>');
  ersetze('h2_inst', /(data-t="h2_inst">)[\s\S]*?(<\/h2>)/, '$1Roboauto <span class="dl-grad">installieren</span>$2');

  // --- ESPCam-Abschnitt vor dem Fuß ---
  ersetze('abschnitt', '<footer class="dl-foot">', ESPCAM_ABSCHNITT + '\n\n  <footer class="dl-foot">');

  // --- Übersetzungen ---
  ersetze('en eyebrow', "<b>MAKING</b>Roboauto · ESP32-S3'", "<b>MAKING</b>Roboauto · ESPCam'");
  ersetze('es eyebrow', "<b>BRICOLAJE</b>Roboauto · ESP32-S3'", "<b>BRICOLAJE</b>Roboauto · ESPCam'");
  ersetze('fr eyebrow', "<b>BRICOLAGE</b>Roboauto · ESP32-S3'", "<b>BRICOLAGE</b>Roboauto · ESPCam'");
  ersetze('en hero_p', 'Flash the Roboauto firmware straight from your browser', 'Flash the firmware for my maker projects straight from your browser');
  ersetze('es hero_p', 'Instala el firmware del Roboauto directamente', 'Instala el firmware de mis proyectos directamente');
  ersetze('fr hero_p', 'Installe le firmware du Roboauto directement', 'Installe le firmware de mes projets directement');
  ersetze('en h2_inst', `h2_inst:'Install <span class="dl-grad">firmware</span>'`, `h2_inst:'Install <span class="dl-grad">Roboauto</span>'`);
  ersetze('es h2_inst', `h2_inst:'Instalar <span class="dl-grad">firmware</span>'`, `h2_inst:'Instalar <span class="dl-grad">Roboauto</span>'`);
  ersetze('fr h2_inst', `h2_inst:'Installer le <span class="dl-grad">firmware</span>'`, `h2_inst:'Installer <span class="dl-grad">Roboauto</span>'`);
  ersetze('en cam', "auto_btn:'Open controls →',", "auto_btn:'Open controls →',\n" + CAM_EN + ',');
  ersetze('es cam', "auto_btn:'Abrir control →',", "auto_btn:'Abrir control →',\n" + CAM_ES + ',');
  ersetze('fr cam', "auto_btn:'Ouvrir le pilotage →',", "auto_btn:'Ouvrir le pilotage →',\n" + CAM_FR + ',');

  // --- Build-Nummer der ESPCam anzeigen ---
  ersetze('var build', "var sprache='de',build=null;", "var sprache='de',build=null,buildCam=null;");
  ersetze('setze ver', "if(v&&build!==null)v.textContent=(W.ver||DE.ver).replace('{0}',build);",
    "if(v&&build!==null)v.textContent=(W.ver||DE.ver).replace('{0}',build);\n" +
    "  var vc=document.getElementById('dl-ver-cam');if(vc&&buildCam!==null)vc.textContent=(W.ver||DE.ver).replace('{0}',buildCam);");
  ersetze('fetch cam', "build=j.build;setze(sprache);}).catch(function(){});",
    "build=j.build;setze(sprache);}).catch(function(){});\n" +
    "fetch('https://prod-byte.github.io/ESPCam/version.json',{cache:'no-store'}).then(function(r){return r.json();}).then(function(j){buildCam=j.build;setze(sprache);}).catch(function(){});");

  return {out: fehler.length ? raw : out, fehler};
}

const ESPCAM_ABSCHNITT = `  <section class="dl-sec" id="espcam">
    <div class="dl-wrap">
      <h2 class="dl-h2 rv" data-t="cam_h2">ESPCam <span class="dl-grad">installieren</span></h2>
      <p class="dl-sub rv"><span data-t="cam_sub">WLAN-Kamera mit Livebild im Browser – AI-Thinker ESP32-CAM per USB anschließen und direkt hier aufspielen.</span> <span id="dl-ver-cam"></span></p>
      <div class="dl-flash">
        <div class="dl-opt rv">
          <h3 data-t="upd_h">🔄 Update</h3>
          <p data-t="cam_upd_p">Neue Firmware aufspielen – <b>gespeicherte WLANs bleiben erhalten.</b></p>
          <esp-web-install-button manifest="https://prod-byte.github.io/ESPCam/manifest-update.json">
            <button slot="activate" data-t="upd_btn">⚡ Update installieren</button>
            <span slot="unsupported" data-t="unsup"><b>Dieser Browser kann das leider nicht.</b> Bitte Chrome oder Edge am Computer verwenden.</span>
            <span slot="not-allowed" data-t="notall"><b>Nur über https möglich.</b></span>
          </esp-web-install-button>
        </div>
        <div class="dl-opt rot rv">
          <h3 data-t="neu_h">🧹 Neuinstallation</h3>
          <p data-t="cam_neu_p">Speicher komplett löschen und neu aufspielen – für <b>neue Kameras</b> oder wenn gar nichts mehr geht. <b>Gespeicherte WLANs sind danach weg.</b></p>
          <esp-web-install-button manifest="https://prod-byte.github.io/ESPCam/manifest-neu.json">
            <button slot="activate" data-t="neu_btn">🗑️ Löschen &amp; neu installieren</button>
            <span slot="unsupported" data-t="unsup"><b>Dieser Browser kann das leider nicht.</b> Bitte Chrome oder Edge am Computer verwenden.</span>
            <span slot="not-allowed" data-t="notall"><b>Nur über https möglich.</b></span>
          </esp-web-install-button>
        </div>
      </div>
      <ol class="dl-schritte" style="margin-top:28px">
        <li class="rv" data-t="cam_s1"><b>Kamera anschließen.</b> ESP32-CAM auf das <b>ESP32-CAM-MB</b>-Board stecken, per USB mit dem Computer verbinden und <b>Chrome</b> oder <b>Edge</b> öffnen.</li>
        <li class="rv" data-t="cam_s2"><b>Anschluss auswählen.</b> Meist „USB-SERIAL CH340“.</li>
        <li class="rv" data-t="cam_s3"><b>Kein Anschluss?</b> Am MB-Board <b>IO0</b> gedrückt halten, kurz <b>RST</b> an der Kamera drücken, IO0 loslassen – nochmal versuchen.</li>
        <li class="rv" data-t="cam_s4"><b>Neu starten.</b> Nach dem Aufspielen einmal <b>RST</b> drücken.</li>
        <li class="rv" data-t="cam_s5"><b>WLAN einrichten</b> (nur beim ersten Mal): Mit dem Handy ins WLAN <b>„ESPCam-Setup“</b>, dein WLAN antippen, Passwort eingeben.</li>
      </ol>
      <div class="dl-auto rv">
        <div>
          <h3 data-t="cam_auto_h">📷 Zur Kamera</h3>
          <p data-t="cam_auto_p">Ist die Kamera im WLAN, öffnest du Livebild und Einstellungen im Browser unter <code>http://espcam.local</code> – Handy oder PC müssen im selben WLAN sein. Quellcode und Anleitung: <a href="https://github.com/ProD-byte/ESPCam" target="_blank" rel="noopener">GitHub</a>.</p>
        </div>
        <a class="dl-btn" href="http://espcam.local" target="_blank" rel="noopener" data-t="cam_auto_btn">Kamera öffnen →</a>
      </div>
    </div>
  </section>`;

const GH = '<a href="https://github.com/ProD-byte/ESPCam" target="_blank" rel="noopener">GitHub</a>';

const CAM_EN = [
  `cam_h2:'Install <span class="dl-grad">ESPCam</span>'`,
  `cam_sub:'Wi-Fi camera with live view in the browser – connect the AI-Thinker ESP32-CAM via USB and flash it right here.'`,
  `cam_upd_p:'Install new firmware – <b>saved Wi-Fi networks are kept.</b>'`,
  `cam_neu_p:'Erase everything and install from scratch – for <b>new cameras</b> or when nothing works anymore. <b>Saved Wi-Fi networks will be gone.</b>'`,
  `cam_s1:'<b>Connect the camera.</b> Put the ESP32-CAM on the <b>ESP32-CAM-MB</b> board, plug it into the computer via USB and open <b>Chrome</b> or <b>Edge</b>.'`,
  `cam_s2:'<b>Select the port.</b> Usually “USB-SERIAL CH340”.'`,
  `cam_s3:'<b>No port?</b> Hold <b>IO0</b> on the MB board, briefly press <b>RST</b> on the camera, release IO0 – try again.'`,
  `cam_s4:'<b>Restart.</b> After flashing, press <b>RST</b> once.'`,
  `cam_s5:'<b>Set up Wi-Fi</b> (first time only): connect your phone to the Wi-Fi <b>“ESPCam-Setup”</b>, tap your Wi-Fi, enter the password.'`,
  `cam_auto_h:'📷 Open the camera'`,
  `cam_auto_p:'Once the camera is on your Wi-Fi, open the live view and settings in your browser at <code>http://espcam.local</code> – phone or PC must be on the same Wi-Fi. Source code and guide: ${GH}.'`,
  `cam_auto_btn:'Open camera →'`,
].join(',\n');

const CAM_ES = [
  `cam_h2:'Instalar <span class="dl-grad">ESPCam</span>'`,
  `cam_sub:'Cámara Wi-Fi con imagen en directo en el navegador – conecta la AI-Thinker ESP32-CAM por USB e instálala aquí mismo.'`,
  `cam_upd_p:'Instala el nuevo firmware – <b>las redes Wi-Fi guardadas se conservan.</b>'`,
  `cam_neu_p:'Borra toda la memoria e instala de nuevo – para <b>cámaras nuevas</b> o cuando ya nada funciona. <b>Las redes Wi-Fi guardadas se borran.</b>'`,
  `cam_s1:'<b>Conecta la cámara.</b> Coloca la ESP32-CAM en la placa <b>ESP32-CAM-MB</b>, conéctala al ordenador por USB y abre <b>Chrome</b> o <b>Edge</b>.'`,
  `cam_s2:'<b>Elige el puerto.</b> Normalmente «USB-SERIAL CH340».'`,
  `cam_s3:'<b>¿No aparece el puerto?</b> Mantén pulsado <b>IO0</b> en la placa MB, pulsa brevemente <b>RST</b> en la cámara, suelta IO0 – y vuelve a intentarlo.'`,
  `cam_s4:'<b>Reinicia.</b> Después de instalar, pulsa <b>RST</b> una vez.'`,
  `cam_s5:'<b>Configura el Wi-Fi</b> (solo la primera vez): conecta el móvil a la red <b>«ESPCam-Setup»</b>, toca tu Wi-Fi e introduce la contraseña.'`,
  `cam_auto_h:'📷 Abrir la cámara'`,
  `cam_auto_p:'Cuando la cámara esté en tu Wi-Fi, abre la imagen en directo y los ajustes en <code>http://espcam.local</code> – el móvil o el PC deben estar en la misma red. Código fuente y guía: ${GH}.'`,
  `cam_auto_btn:'Abrir cámara →'`,
].join(',\n');

const CAM_FR = [
  `cam_h2:'Installer <span class="dl-grad">ESPCam</span>'`,
  `cam_sub:'Caméra Wi-Fi avec image en direct dans le navigateur – branche l’AI-Thinker ESP32-CAM en USB et installe-la directement ici.'`,
  `cam_upd_p:'Installe le nouveau firmware – <b>les Wi-Fi enregistrés sont conservés.</b>'`,
  `cam_neu_p:'Efface toute la mémoire et réinstalle – pour les <b>nouvelles caméras</b> ou quand plus rien ne marche. <b>Les Wi-Fi enregistrés seront effacés.</b>'`,
  `cam_s1:'<b>Branche la caméra.</b> Place l’ESP32-CAM sur la carte <b>ESP32-CAM-MB</b>, relie-la à l’ordinateur en USB et ouvre <b>Chrome</b> ou <b>Edge</b>.'`,
  `cam_s2:'<b>Choisis le port.</b> En général « USB-SERIAL CH340 ».'`,
  `cam_s3:'<b>Pas de port ?</b> Maintiens <b>IO0</b> sur la carte MB, appuie brièvement sur <b>RST</b> sur la caméra, relâche IO0 – puis réessaie.'`,
  `cam_s4:'<b>Redémarre.</b> Après l’installation, appuie une fois sur <b>RST</b>.'`,
  `cam_s5:'<b>Configure le Wi-Fi</b> (la première fois seulement) : connecte ton téléphone au Wi-Fi <b>« ESPCam-Setup »</b>, touche ton Wi-Fi, saisis le mot de passe.'`,
  `cam_auto_h:'📷 Ouvrir la caméra'`,
  `cam_auto_p:'Une fois la caméra sur ton Wi-Fi, ouvre l’image en direct et les réglages à l’adresse <code>http://espcam.local</code> – le téléphone ou le PC doivent être sur le même Wi-Fi. Code source et guide : ${GH}.'`,
  `cam_auto_btn:'Ouvrir la caméra →'`,
].join(',\n');
