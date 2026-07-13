# Weather App Backend

A high-performance REST API and WebSocket backend for a weather application, built in C++ using the Drogon framework and PostgreSQL.

## Tech Stack

- **Language:** C++20
- **Framework:** Drogon
- **Database:** PostgreSQL 17
- **Auth:** JWT (HS256) with PBKDF2-SHA256 password hashing
- **External API:** OpenWeatherMap
- **Build System:** CMake

## Project Structure

```
my_cpp_backend/
├── main.cc                              # Application entry point
├── config.json                          # Drogon configuration
├── CMakeLists.txt                       # Build configuration
├── API_TESTING.md                       # Postman testing guide
├── migrations/
│   └── 001_init.sql                     # Database schema
├── common/
│   ├── Response.h                       # Standardized JSON responses
│   ├── JwtUtils.h / .cc                 # JWT encode/decode
│   ├── Bcrypt.h / .cc                   # Password hashing
│   └── Utils.h                          # String utilities
├── filters/
│   ├── JwtAuthFilter.h / .cc            # JWT token validation
│   └── CorsMiddleware.h / .cc           # CORS headers
├── services/
│   ├── AuthService.h / .cc              # Auth business logic
│   ├── WeatherService.h / .cc           # Weather data + caching
│   └── OpenWeatherMapClient.h / .cc     # External API client
├── controllers/api/v1/
│   ├── Auth.h / .cc                     # Auth endpoints
│   ├── Weather.h / .cc                  # Weather endpoints
│   ├── Favorites.h / .cc               # Favorites endpoints
│   └── History.h / .cc                 # Search history endpoints
└── websocket/
    └── WeatherStream.h / .cc           # Live weather WebSocket
```

## Getting Started

### Prerequisites

- C++17 or newer compiler (Clang/GCC)
- CMake 3.5+
- PostgreSQL 17
- Drogon framework installed
- OpenSSL
- OpenWeatherMap API key (free tier)

### Installation

```bash
# Clone the repository
git clone https://github.com/EmmanuelMunanka38/weather_Api.git
cd weather_Api

# Build
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.ncpu)
```

### Database Setup

```bash
# Start PostgreSQL
brew services start postgresql@17

# Create database and tables
psql -U postgres -f migrations/001_init.sql
```

### Configuration

1. Edit `config.json` to match your PostgreSQL credentials:
   ```json
   "db_clients": [{
       "dbname": "weather_app",
       "user": "postgres",
       "passwd": "your_password"
   }]
   ```

2. Add your OpenWeatherMap API key in `services/OpenWeatherMapClient.cc`:
   ```cpp
   apiKey_ = "your_api_key_here";
   ```

### Run

```bash
./build/my_cpp_backend
```

Server starts at `http://localhost:8080`.

## API Endpoints

### Public

| Method | Path | Description |
|--------|------|-------------|
| POST | `/api/v1/auth/register` | Register a new user |
| POST | `/api/v1/auth/login` | Login and receive JWT |
| WS | `/ws/weather/{city}` | WebSocket weather stream |

### Protected (requires `Authorization: Bearer <token>`)

| Method | Path | Description |
|--------|------|-------------|
| GET | `/api/v1/auth/profile` | Get current user profile |
| GET | `/api/v1/weather/current/{city}` | Get current weather |
| GET | `/api/v1/weather/forecast/{city}` | Get 5-day forecast |
| GET | `/api/v1/weather/search?q={query}` | Search cities |
| GET | `/api/v1/favorites` | List favorite cities |
| POST | `/api/v1/favorites` | Add city to favorites |
| DELETE | `/api/v1/favorites/{city_id}` | Remove from favorites |
| GET | `/api/v1/history` | Get search history |
| POST | `/api/v1/history` | Record a search |
| DELETE | `/api/v1/history` | Clear search history |

## Testing

See `API_TESTING.md` for detailed Postman instructions with request bodies and expected responses for every endpoint.

## License

MIT
