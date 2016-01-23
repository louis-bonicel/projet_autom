#ifndef __TACHY_H
#define __TACHY_H



/// @var static const float K_TACHY
/// @brief Constante pour convertir une valeur d'ADC en tours par minutes.
/// Vitesse en RPM = Valeur d'ADC / K_TACHY
static const float K_TACHY = 0.518;

/// @var static const float K_TACHY_TO_RPM
/// @brief Constante pour convertir une valeur d'ADC en tours par minutes.
/// K_TACHY_TO_RPM = 1 / K_TACHY
static const float K_TACHY_TO_RPM = 1.93003663004;

void Tachy_to_RPM( int16_t value_to_convert , volatile int16_t * speed_rpm );
void Tachy_Config( void );

#endif // __TACHY_H
