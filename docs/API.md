# Dashboard API Reference

The D1 Mini runs a web server on port 80 with two endpoints.

## Endpoints

### GET /

Returns the full HTML dashboard page with embedded JavaScript for live charting.

### GET /data

Returns current and historical sensor data as JSON.

**Response:**

```json
{
  "temp": 24.1,
  "hum": 55.0,
  "tempHistory": [24.0, 24.1, 24.1, 24.2, ...],
  "humHistory": [53.0, 54.0, 55.0, 55.0, ...]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `temp` | float | Current temperature in Celsius (1 decimal) |
| `hum` | float | Current relative humidity in % (1 decimal) |
| `tempHistory` | float[] | Last 120 temperature readings (oldest first) |
| `humHistory` | float[] | Last 120 humidity readings (oldest first) |

**Notes:**

- History stores up to 120 readings in a circular buffer
- At 2-second intervals, this covers approximately 4 minutes
- History arrays grow from 0 to 120 as readings accumulate
- Readings are oldest-first (index 0 = oldest)

## Usage Examples

### curl

```bash
curl http://10.178.130.63/data
```

### Python

```python
import requests, time

while True:
    r = requests.get("http://10.178.130.63/data")
    d = r.json()
    print(f"Temp: {d['temp']}°C  Humidity: {d['hum']}%")
    time.sleep(2)
```

### Serial Monitor

All readings are also printed to serial at 115200 baud:

```
Temp: 24.1°C  |  Humidity: 55.0%
```

## Logging to CSV

Use the `/data` endpoint to log readings to a file:

```python
import requests, csv, time
from datetime import datetime

with open("readings.csv", "a", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["timestamp", "temp_c", "humidity"])
    while True:
        d = requests.get("http://10.178.130.63/data").json()
        writer.writerow([datetime.now().isoformat(), d["temp"], d["hum"]])
        f.flush()
        time.sleep(2)
```
