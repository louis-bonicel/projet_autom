#include "PrjDefinitions.h"


/**
 * @brief Initialisation de structure Data.
 * @param data La structure a initialiser.
 */
void Data_Init( t_Data * data )
{
	data -> speed_encoder			= 0;
	data -> speed_tachy				= 0;
	data -> speed					= 0;
	data -> consigneReceived		= 0;
	data -> potardValue				= 0;
}


/**
 * @brief Initialisation de structure consigne.
 * @param consigne La structure a initialiser.
 */
void Consigne_Init( t_ConsigneReceived * consigne )
{
	consigne -> mode		= NORMAL;
	consigne -> start_point = 0;
	consigne -> end_point	= 0;
}
