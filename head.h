#pragma once

struct place {
    const char* city;
    const char* country;
    inline place() {}
    inline place(const char* city, const char* country) :
        city(city), country(country) {}
};