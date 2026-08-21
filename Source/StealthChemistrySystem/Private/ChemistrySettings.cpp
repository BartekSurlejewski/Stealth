#include "ChemistrySettings.h"

UChemistrySettings::UChemistrySettings()
{
	RadialQueryCollisionChannels.Add(ECC_WorldDynamic);
	RadialQueryCollisionChannels.Add(ECC_Pawn);
	RadialQueryCollisionChannels.Add(ECC_PhysicsBody);
	RadialQueryCollisionChannels.Add(ECC_WorldStatic);
}
