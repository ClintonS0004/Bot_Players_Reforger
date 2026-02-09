modded class ArmaReforgerScripted
{
	override event void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		
		RplMode mode = RplSession.Mode();
        if (mode == RplMode.Client)
        {
            return;
        }
		
		// Incase someone places it by the workdesk
		if( world.IsEditMode()) return; //{D0F84AE355C0CFDE}Prefabs/CLINTON_EntitySpawnerEntity_Networked.et
		/*
		CLINTON_EntitySpawnerEntity worldEnt = CLINTON_EntitySpawnerEntity.Cast(GetGame().GetWorld().FindEntityByName("CLINTON_EntitySpawnerEntity_This"));
		
		if( !worldEnt )
		{  // this is to be expected
			ref Resource reso = Resource.Load("{0D9B08390A093235}Prefabs/CLINTON_EntitySpawnerEntity_This.et");
			
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = "0 0 0";
			CLINTON_EntitySpawnerEntity.Cast(GetGame().SpawnEntityPrefab(
					reso,
					GetGame().GetWorld(),
					spawnParams
			));
		}*/
		
		
		CLINTON_BotWaypointManagerEntity movementEnt = CLINTON_BotWaypointManagerEntity.Cast(world.FindEntityByName("CLINTON_BotWaypointManagerEntity"));
	
		if( !movementEnt )
		{  // this is to be expected
			ref Resource reso = Resource.Load("{796941AE041E3F18}Prefabs/CLINTON_BotWaypointManagerEntity.et");
			
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform[3] = "0 0 0";
			GetGame().SpawnEntityPrefab(
					reso,
					world,
					spawnParams
			);
		}
	}
}