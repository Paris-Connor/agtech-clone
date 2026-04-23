#!/usr/bin/env python3
"""AG Tech - Plant Camera Server
Captures images from a USB camera/microscope and serves them over HTTP.
The dashboard pulls images from http://localhost:8082/capture

Usage: python3 server.py [camera_index]
  camera_index: 0 = first camera (default), 1 = second camera, etc.
"""

import sys
import time
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler
import cv2

CAMERA_INDEX = int(sys.argv[1]) if len(sys.argv) > 1 else 0
PORT = 8082
CAPTURE_INTERVAL = 5  # seconds between auto-captures

latest_jpg = None
lock = threading.Lock()

def capture_loop():
    global latest_jpg
    cap = cv2.VideoCapture(CAMERA_INDEX)
    if not cap.isOpened():
        print(f"ERROR: Cannot open camera {CAMERA_INDEX}")
        return

    w = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    h = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    print(f"Camera {CAMERA_INDEX} opened: {w}x{h}")

    while True:
        ret, frame = cap.read()
        if ret:
            _, jpg = cv2.imencode('.jpg', frame, [cv2.IMWRITE_JPEG_QUALITY, 85])
            with lock:
                latest_jpg = jpg.tobytes()
        time.sleep(CAPTURE_INTERVAL)

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path.startswith('/capture'):
            with lock:
                jpg = latest_jpg
            if jpg:
                self.send_response(200)
                self.send_header('Content-Type', 'image/jpeg')
                self.send_header('Content-Length', str(len(jpg)))
                self.send_header('Access-Control-Allow-Origin', '*')
                self.send_header('Cache-Control', 'no-cache')
                self.end_headers()
                self.wfile.write(jpg)
            else:
                self.send_response(503)
                self.send_header('Content-Type', 'text/plain')
                self.end_headers()
                self.wfile.write(b'No image captured yet')

        elif self.path == '/status':
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Access-Control-Allow-Origin', '*')
            self.end_headers()
            has_img = 'true' if latest_jpg else 'false'
            self.wfile.write(f'{{"status":"ok","camera":{CAMERA_INDEX},"hasImage":{has_img}}}'.encode())

        else:
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write(b'''<!DOCTYPE html><html><head>
<title>AG Tech Plant Cam</title>
<style>body{font-family:sans-serif;background:#f0f7f0;text-align:center;padding:20px}
img{max-width:100%;border-radius:12px;border:2px solid #c8e0c8}h1{color:#2d6a2d}</style>
</head><body><h1>AG Tech Plant Cam</h1>
<p><img src="/capture" id="img"></p>
<script>setInterval(()=>{document.getElementById('img').src='/capture?'+Date.now()},5000);</script>
</body></html>''')

    def log_message(self, format, *args):
        pass  # quiet logs

if __name__ == '__main__':
    print(f"--- AG Tech Plant Camera Server ---")
    print(f"Starting camera {CAMERA_INDEX}...")

    t = threading.Thread(target=capture_loop, daemon=True)
    t.start()

    # Wait for first capture
    for _ in range(20):
        time.sleep(0.5)
        if latest_jpg:
            break

    print(f"Server running on http://localhost:{PORT}")
    print(f"  Snapshot: http://localhost:{PORT}/capture")
    print(f"  Preview:  http://localhost:{PORT}/")
    print(f"  Status:   http://localhost:{PORT}/status")
    print(f"Press Ctrl+C to stop")

    server = HTTPServer(('0.0.0.0', PORT), Handler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping...")
        server.server_close()
