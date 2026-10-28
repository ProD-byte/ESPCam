# Nimmt den Rohinhalt der Basteln-Seite aus dem Browser entgegen und speichert
# ihn als Backup (nur lokal auf 127.0.0.1, beendet sich nach einer Datei).
import sys
from http.server import BaseHTTPRequestHandler, HTTPServer

ziel = sys.argv[1]


class Empfang(BaseHTTPRequestHandler):
    def _cors(self):
        self.send_header("Access-Control-Allow-Origin", "https://daveslab.de")
        self.send_header("Access-Control-Allow-Methods", "POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Content-Type")
        self.send_header("Access-Control-Allow-Private-Network", "true")

    def do_OPTIONS(self):
        self.send_response(204)
        self._cors()
        self.end_headers()

    def do_POST(self):
        daten = self.rfile.read(int(self.headers["Content-Length"]))
        with open(ziel, "wb") as f:
            f.write(daten)
        self.send_response(200)
        self._cors()
        self.end_headers()
        self.wfile.write(b"ok")
        print(f"gespeichert: {ziel} ({len(daten)} Bytes)", flush=True)
        raise SystemExit


HTTPServer(("127.0.0.1", 8766), Empfang).serve_forever()
