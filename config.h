// config.h
#include "params.h"
#ifndef CONFIG_H
#define CONFIG_H
/* *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *                                                      CONFIG                                                 *
 * *-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * 1) choose airport airport international traffic ratio configuration
 * 
 * each entry defines:
 *   - airport name
 *   - location (city / country)
 *   - international percentage (integer, 0–100)
 *
 * options:
 *   - JOHN_F_KENNEDY_INTERNATIONAL     56% international,     ~35.08M international / 62.5M total
 *   - LONDON_HEATHROW                  95% international,     ~74.96M international / 79.2M total
 *   - DUBAI_INTERNATION                99% international,     domestic negligibale
 *   - HARTSFIELD_JACKSON_ATLANTA       15% international,     ~15.7M international / 104.7M total
 *   - SAO_PAULO_GUARULHOS              35% international,     1.3M international / 3.7M total
 *
 */
static const AirportParameters AIRPORT = AIRPORTS[SAO_PAULO_GUARULHOS];
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
 *
 * 2) choose basic simulation airport parameters
 * 
 */
static const int SIM_DURATION               = 300;  // seconds
static const int N_TRACKS                   = 3;    // number of tracks
static const int N_GATES                    = 5;    // number of gates
static const int N_TOWER_MAX_OPERATIONS       = 2;    // max number of simultaneous operations the tower can do
static const int MAX_N_PLANES               = 1;    // maximum number of planes #TODO
static const int TIME_TILL_CRITICAL_STATE   = 60;   // time till critical state in seconds
static const int TIME_TILL_CRASH            = 90;   // time till crash in seconds
static const int WAITING_TIMEOUT            = 60;//
/*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*/
#endif /* CONFIG_H */
