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
  "lux": 12500.0,
  "soil": 65.0,
  "status": "ok",
  "tempHistory": [24.0, 24.1, ...],
  "humHistory": [53.0, 54.0, ...],
  "luxHistory": [12000.0, 12500.0, ...],
  "soilHistory": [64.0, 65.0, ...]
}
```

| Field | Type | Description |
|-------|------|-------------|
| `temp` | float | Temperature in Celsius |
| `hum` | float | Relative humidity % |
| `lux` | float | Light intensity in lux (-1 if sensor not connected) |
| `soil` | float | Soil moisture % (-1 if sensor not connected) |
| `status` | string | "ok", "warn", or "danger" |
| `tempHistory` | float[] | Last 120 temperature readings (oldest first) |
| `humHistory` | float[] | Last 120 humidity readings |
| `luxHistory` | float[] | Last 120 light readings (-1 if no sensor) |
| `soilHistory` | float[] | Last 120 soil readings (-1 if no sensor) |

**Notes:**

- History stores up to 120 readings in a circular buffer
- At 2-second intervals, this covers approximately 4 minutes
- Sensors that are not connected return -1 and are excluded from status evaluation
- Soil sensor auto-detects: if A0 reads 1023 (open circuit), it's marked as not present

## ThingSpeak Cloud Integration

The D1 pushes data to ThingSpeak every 20 seconds (free tier minimum is 15s).

| ThingSpeak Field | Sensor |
|-----------------|--------|
| field1 | Temperature (C) |
| field2 | Humidity (%) |
| field3 | Light (lux) |
| field4 | Soil Moisture (%) |

**Read latest from ThingSpeak:**
```
GET https://api.thingspeak.com/channels/{CHANNEL_ID}/feeds/last.json?api_key={READ_KEY}
```

**Read history:**
```
GET https://api.thingspeak.com/channels/{CHANNEL_ID}/feeds.json?api_key={READ_KEY}&results=120
```

## Status Thresholds

| Sensor | OK | Warning | Danger |
|--------|-----------|-----------------|-------------|
| Temperature | 10-30 C | 5-10 / 30-35 C | <5 / >35 C |
| Humidity | 30-70% | 20-30 / 70-80% | <20 / >80% |
| Light | 500-50k lux | <500 / >50k | <200 / >80k |
| Soil | 25-80% | <25 / >80% | <15 / >90% |
