# Weather App Backend — API Testing Guide

## Prerequisites

### 1. Install Dependencies

```bash
# macOS (Homebrew)
brew install drogon jsoncpp openssl postgresql@17 brotli

# Verify Drogon is installed
drogon_ctl version
```

### 2. Set Up PostgreSQL Database

```bash
# Start PostgreSQL
brew services start postgresql@17

# Create the database and tables
psql -U postgres -f migrations/001_init.sql
```

### 3. Set Your OpenWeatherMap API Key

Get a free key at [https://openweathermap.org/api](https://openweathermap.org/api)

Edit `services/OpenWeatherMapClient.cc` line 13:
```cpp
apiKey_ = "your_actual_api_key_here";
```

### 4. Build & Run

```bash
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.ncpu)
./my_cpp_backend
```

Server starts at `http://localhost:8080`

---

## Postman Setup

Import the environment variables below into Postman:

| Variable | Value |
|----------|-------|
| `base_url` | `http://localhost:8080` |
| `token` | *(set automatically after login)* |

---

## API Endpoints

### Auth

#### Register
```
POST {{base_url}}/api/v1/auth/register
Content-Type: application/json

{
  "email": "john@example.com",
  "username": "johndoe",
  "password": "secret123"
}
```

**Expected Response (201):**
```json
{
  "status": "success",
  "message": "User registered successfully",
  "data": {
    "id": 1,
    "email": "john@example.com",
    "username": "johndoe",
    "token": "eyJhbGciOiJIUzI1NiIs..."
  }
}
```

> Copy the `token` value — you'll need it for all authenticated requests.

---

#### Login
```
POST {{base_url}}/api/v1/auth/login
Content-Type: application/json

{
  "email": "john@example.com",
  "password": "secret123"
}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "message": "Login successful",
  "data": {
    "id": 1,
    "email": "john@example.com",
    "username": "johndoe",
    "token": "eyJhbGciOiJIUzI1NiIs..."
  }
}
```

---

#### Get Profile
```
GET {{base_url}}/api/v1/auth/profile
Authorization: Bearer {{token}}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "data": {
    "id": 1,
    "email": "john@example.com",
    "username": "johndoe",
    "created_at": "2026-07-13 12:00:00"
  }
}
```

---

### Weather

#### Get Current Weather
```
GET {{base_url}}/api/v1/weather/current/London
Authorization: Bearer {{token}}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "data": {
    "source": "api",
    "weather": {
      "id": 1,
      "city_id": 1,
      "temperature": 22.5,
      "feels_like": 21.8,
      "humidity": 65,
      "pressure": 1013,
      "wind_speed": 3.5,
      "wind_direction": 180,
      "description": "partly cloudy",
      "icon": "02d"
    }
  }
}
```

---

#### Get Forecast
```
GET {{base_url}}/api/v1/weather/forecast/Paris?days=3
Authorization: Bearer {{token}}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "data": {
    "city": "Paris",
    "days": 3,
    "forecasts": [
      {
        "date": "2026-07-15 12:00:00",
        "temp_min": 18.2,
        "temp_max": 25.1,
        "humidity": 70,
        "description": "light rain",
        "icon": "10d"
      }
    ]
  }
}
```

---

#### Search Cities
```
GET {{base_url}}/api/v1/weather/search?q=Tokyo
Authorization: Bearer {{token}}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "data": [
    {
      "name": "Tokyo",
      "country": "JP",
      "latitude": 35.6895,
      "longitude": 139.6917,
      "state": "Tokyo"
    }
  ]
}
```

---

### Favorites

#### List Favorites
```
GET {{base_url}}/api/v1/favorites
Authorization: Bearer {{token}}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "data": {
    "favorites": [
      {
        "id": 1,
        "city_id": 1,
        "city_name": "London",
        "country_code": "GB",
        "latitude": 51.5074,
        "longitude": -0.1278
      }
    ],
    "count": 1
  }
}
```

---

#### Add Favorite
```
POST {{base_url}}/api/v1/favorites
Authorization: Bearer {{token}}
Content-Type: application/json

{
  "city_name": "London",
  "country_code": "GB"
}
```

**Expected Response (201):**
```json
{
  "status": "success",
  "message": "Added to favorites",
  "data": {
    "id": 1,
    "city_id": 1,
    "city_name": "London",
    "country_code": "GB"
  }
}
```

---

#### Remove Favorite
```
DELETE {{base_url}}/api/v1/favorites/1
Authorization: Bearer {{token}}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "data": {
    "message": "Removed from favorites"
  }
}
```

---

### Search History

#### Add Search
```
POST {{base_url}}/api/v1/history
Authorization: Bearer {{token}}
Content-Type: application/json

{
  "city_name": "Berlin",
  "country_code": "DE"
}
```

**Expected Response (201):**
```json
{
  "status": "success",
  "message": "Search recorded",
  "data": {
    "id": 1,
    "city_id": 2,
    "city_name": "Berlin"
  }
}
```

---

#### Get History
```
GET {{base_url}}/api/v1/history
Authorization: Bearer {{token}}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "data": {
    "history": [
      {
        "id": 1,
        "city_id": 2,
        "city_name": "Berlin",
        "country_code": "DE",
        "searched_at": "2026-07-13 12:05:00"
      }
    ],
    "count": 1
  }
}
```

---

#### Clear History
```
DELETE {{base_url}}/api/v1/history
Authorization: Bearer {{token}}
```

**Expected Response (200):**
```json
{
  "status": "success",
  "data": {
    "message": "Search history cleared"
  }
}
```

---

### WebSocket (Live Weather Stream)

Connect via WebSocket client (Postman WS tab or `wscat`):

```
ws://localhost:8080/ws/weather/London
```

**On connect you'll receive:**
```json
{"status":"connected","message":"Send {\"action\":\"subscribe\",\"city\":\"CityName\"} to subscribe to weather updates"}
```

**Subscribe to a city:**
```json
{"action":"subscribe","city":"London"}
```

**Expected response:**
```json
{"status":"subscribed","city":"London"}
```

---

## Testing Sequence (Copy-Paste Order)

1. `POST /api/v1/auth/register` — create account
2. Copy `token` from response
3. Set `Authorization: Bearer <token>` on all remaining requests
4. `GET /api/v1/weather/current/NewYork` — fetch weather
5. `POST /api/v1/history` — record search
6. `POST /api/v1/favorites` — save city
7. `GET /api/v1/favorites` — list saved cities
8. `GET /api/v1/history` — view search history
9. `DELETE /api/v1/favorites/1` — remove a favorite
10. `DELETE /api/v1/history` — clear history
11. Connect to WebSocket `ws://localhost:8080/ws/weather/Tokyo` for live updates
