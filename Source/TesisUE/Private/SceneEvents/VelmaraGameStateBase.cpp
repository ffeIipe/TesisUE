#include "SceneEvents/VelmaraGameStateBase.h"
#include "Entities/Entity.h"


//void AVelmaraGameStateBase::UpdateEntityState(AEntity* DeadEntity)
//{
//	const FEntityMementoState DeadEntityState = DeadEntity->Execute_GetMementoComponent(DeadEntity)->CaptureOwnerState();
//	if (WorldEntityStates.Find(DeadEntity->GetUniqueSaveID()))
//	{
//		WorldEntityStates[DeadEntity->GetUniqueSaveID()] = DeadEntityState;
//	}
//
//	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Orange, DeadEntity->GetName() + " updated.  [" + DeadEntity->GetUniqueSaveID().ToString() + "]");
//}