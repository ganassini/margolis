// params.h
#ifndef PARAMS_H
#define PARAMS_H

typedef enum {
    JOHN_F_KENNEDY_INTERNATIONAL    = 0,
    LONDON_HEATHROW                 = 1,
    DUBAI_INTERNATIONAL             = 2,
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
        // TODO: source
    },
    {   
        LONDON_HEATHROW,
        "London Heathrow (LHR)",
        "London, UK",
        95, // ~74.96M international / 79.2M total
        // TODO: source
    },
    {
        DUBAI_INTERNATIONAL,
        "Dubai International (DXB)",
        "Dubai, UAE",
        99, // domestic negligible
        // TODO: source
    },
    {
        HARTSFIELD_JACKSON_ATLANTA,
        "Hartsfield-Jackson Atlanta (ATL)",
        "Atlanta, USA",
        15, // ~15.7M international / 104.7M total
        // TODO: source
    },
    {
        SAO_PAULO_GUARULHOS,
        "São Paulo–Guarulhos (GRU)",
        "São Paulo, Brazil",
        35, // 1.3M international / 3.7M total
        // TODO: source
    }
};

static const int NUM_AIRPORTS = sizeof(AIRPORTS) / sizeof(AIRPORTS[0]);

#endif /* PARAMS_H */
