CREATE DATABASE weather_app;

\c weather_app;

CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    email VARCHAR(255) UNIQUE NOT NULL,
    username VARCHAR(100) NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT NOW(),
    updated_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE weather_cities (
    id SERIAL PRIMARY KEY,
    city_name VARCHAR(255) NOT NULL,
    country_code VARCHAR(10),
    latitude DOUBLE PRECISION,
    longitude DOUBLE PRECISION,
    openweathermap_id INTEGER,
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(city_name, country_code)
);

CREATE TABLE weather_readings (
    id SERIAL PRIMARY KEY,
    city_id INTEGER REFERENCES weather_cities(id) ON DELETE CASCADE,
    temperature DOUBLE PRECISION,
    feels_like DOUBLE PRECISION,
    humidity INTEGER,
    pressure INTEGER,
    wind_speed DOUBLE PRECISION,
    wind_direction INTEGER,
    description VARCHAR(255),
    icon VARCHAR(50),
    fetched_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE weather_forecasts (
    id SERIAL PRIMARY KEY,
    city_id INTEGER REFERENCES weather_cities(id) ON DELETE CASCADE,
    forecast_date DATE NOT NULL,
    temp_min DOUBLE PRECISION,
    temp_max DOUBLE PRECISION,
    humidity INTEGER,
    description VARCHAR(255),
    icon VARCHAR(50),
    fetched_at TIMESTAMP DEFAULT NOW()
);

CREATE TABLE user_favorites (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
    city_id INTEGER REFERENCES weather_cities(id) ON DELETE CASCADE,
    created_at TIMESTAMP DEFAULT NOW(),
    UNIQUE(user_id, city_id)
);

CREATE TABLE search_history (
    id SERIAL PRIMARY KEY,
    user_id INTEGER REFERENCES users(id) ON DELETE CASCADE,
    city_id INTEGER REFERENCES weather_cities(id) ON DELETE CASCADE,
    searched_at TIMESTAMP DEFAULT NOW()
);
