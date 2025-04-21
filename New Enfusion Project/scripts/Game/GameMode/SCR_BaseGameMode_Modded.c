modded class ArmaReforgerScripted
{
	override event void OnWorldPostProcess(World world)
	{
		super.OnWorldPostProcess(world);
		
		// Incase someone places it by the workdesk
		if( world.IsEditMode()) return;
		
		CLINTON_BotsManagerEntity worldEnt = CLINTON_BotsManagerEntity.Cast(world.FindEntityByName("CLINTON_BotsManagerEntity"));
		
		if( !worldEnt )
		{  // this is to be expected
			ref Resource reso = Resource.Load("{875AF9006032BBD8}Prefabs/CLINTON_BotsManagerEntity.et");
			
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