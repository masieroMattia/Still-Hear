#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SaveIdComponent.generated.h"

/**
 * This component enables serialization of an Actor by providing a stable unique identifier (GUID).
 * All variables marked as SaveGame in the Actor will be serialized when saving/loading the game.
 * All actors that need to be saved should have this component to provide a stable unique id
 * Only Actor with this component will be considered for saving/loading
 * 
 * BUGS/Limitations: The first time an actor is created, the ID is not generated, you have to 
 * generate it manually in editor by clicking the "Generate ID" button in the component details
 */
UCLASS(ClassGroup=(Save), meta=(BlueprintSpawnableComponent))
class STILLHEAR_API USaveIdComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USaveIdComponent();

	UFUNCTION(BlueprintPure, Category="Save")
	FGuid GetSaveId() const;
	
	UFUNCTION(CallInEditor, Category="Save")
	void GenerateID();
	
	/**
    * Regenerate a new unique id (only in editor)
    * Create a button in editor to call this function
    * Warning: this will break existing saves for the actor
    */
	UFUNCTION(CallInEditor, Category="Save")
	void RegenerateID();

#if WITH_EDITOR
	/**
	*Directly from Unreal Engine documentation:
    * Called after importing property values for this object (paste, duplicate or .t3d import)
    * Allow the object to perform any cleanup for properties which shouldn't be duplicated or
    * are unsupported by the script serialization
    */
	virtual void PostEditImport() override; 
	
#endif

	virtual void OnComponentCreated() override;
	virtual void PostLoad() override;

private:
	
	// The unique id for saving/loading
	UPROPERTY(VisibleAnywhere,Category = "Save")
	FGuid SaveId;
	
	// To detect if the object was just created or loaded from disk
	UPROPERTY(VisibleDefaultsOnly,Category = "Save")
	bool bNull = false;
	
	/**
	 *Ensure the id is valid, optionally forcing a new one.
	 *This method effectively generates a new GUID if needed
	*/
	void EnsureId(bool bForceNew);
};
