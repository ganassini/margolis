// params.h - Remove the #include "config.h" line
#ifndef PARAMS_H
#define PARAMS_H

typedef enum {
    JOHN_F_KENNEDY_INTERNATIONAL    = 0,
    LONDON_HEATHROW                 = 1,
    DUBAI_INTERNATION               = 2,
    HARTSFIELD_JACKSON_ATLANTA      = 3,
    SAO_PAULO_GUARULHOS             = 4
} Name;

typedef struct {
    Name name;
    const char *long_name;
    const char *location;
    int international_flights_percentage;
    // const char *source;
} AirportParameters;

static const AirportParameters AIRPORTS[] = {
    {
        JOHN_F_KENNEDY_INTERNATIONAL,
        "John F. Kennedy International (JFK)",
        "New York, USA",
        56, // ~35.08M international / 62.5M total
    },
    {   
        LONDON_HEATHROW,
        "London Heathrow (LHR)",
        "London, UK",
        95, // ~74.96M international / 79.2M total
    },
    {
        DUBAI_INTERNATION,
        "Dubai International (DXB)",
        "Dubai, UAE",
        99, // domestic negligible
    },
    {
        HARTSFIELD_JACKSON_ATLANTA,
        "Hartsfield-Jackson Atlanta (ATL)",
        "Atlanta, USA",
        15, // ~15.7M international / 104.7M total
    },
    {
        SAO_PAULO_GUARULHOS,
        "São Paulo–Guarulhos (GRU)",
        "São Paulo, Brazil",
        35, // 1.3M international / 3.7M total
    }
};

static const int NUM_AIRPORTS = sizeof(AIRPORTS) / sizeof(AIRPORTS[0]);

#endif /* PARAMS_H */
