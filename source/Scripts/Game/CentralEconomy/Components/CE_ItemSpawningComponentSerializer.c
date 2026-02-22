class CE_ItemSpawningComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return CE_ItemSpawningComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const CE_ItemSpawningComponent spawningComp = CE_ItemSpawningComponent.Cast(component);

		context.WriteValue("version", 1);
		context.WriteValue("readyForItem", spawningComp.IsReadyForItem());
		context.WriteValue("itemTaken", spawningComp.HasSpawnedItemBeenTaken());
		context.WriteValue("currentResetTime", spawningComp.GetCurrentSpawnerResetTime());
		context.WriteValue("spawnedItemUUID", spawningComp.GetSpawnedItemUUID());
		context.WriteValue("spawnerUUID", spawningComp.GetSpawnerUUID());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		CE_ItemSpawningComponent spawningComp = CE_ItemSpawningComponent.Cast(component);
		
		CE_Item spawnedItem;
		CE_Spawner spawner;
		CE_ItemSpawningSystem spawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(owner);
		
		int version;
		context.Read(version);

		bool readyForItem;
		bool itemTaken;
		int currentResetTime;
		UUID spawnedItemUUID;
		UUID spawnerUUID;
		
		if (context.Read(readyForItem))
			spawningComp.SetReadyForItem(readyForItem);
		
		if (context.Read(itemTaken))
			spawningComp.SetHasSpawnedItemBeenTaken(itemTaken);
		
		if (context.Read(currentResetTime))
			spawningComp.SetCurrentSpawnerResetTime(currentResetTime);
		
		if (context.Read(spawnedItemUUID))
		{
			spawningComp.SetSpawnedItemUUID(spawnedItemUUID);
			
			if (!spawnedItemUUID.IsNull())
			{
				if (spawningSystem)
				{
					array<ref CE_Spawner> spawners = spawningSystem.GetSpawners();
					
					if (spawners && !spawners.IsEmpty())
					{
						spawnedItem = spawningSystem.FindItemByUUID(spawnedItemUUID);
						if (spawnedItem)
							spawningComp.SetItemSpawned(spawnedItem);
					}
				}
			}
		}
		
		if (context.Read(spawnerUUID))
		{
			spawningComp.SetSpawnerUUID(spawnerUUID);
			
			if (!spawnerUUID.IsNull())
			{
				if (spawningSystem)
				{
					spawner = spawningSystem.FindSpawnerByUUID(spawnerUUID);
				}
			}
		}
		
		if (itemTaken)
		{
			CE_SpawnerTimingSystem spawnerTimingSystem = CE_SpawnerTimingSystem.GetByEntityWorld(owner);
			if (spawnerTimingSystem)
			{
				spawnerTimingSystem.RegisterSpawner(spawningComp);
			}
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void OnSpawnablesAvailable(Managed instance, PersistenceDeferredDeserializeTask task, bool expired, Managed context)
	{
		
	}
}
