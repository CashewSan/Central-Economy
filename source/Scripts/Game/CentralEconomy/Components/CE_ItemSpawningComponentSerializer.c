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
		context.WriteValue("itemSpawned", spawningComp.GetItemSpawned());
		context.WriteValue("currentResetTime", spawningComp.GetCurrentSpawnerResetTime());
		context.WriteValue("spawnerUUID", spawningComp.GetSpawnerUUID());
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		CE_ItemSpawningComponent spawningComp = CE_ItemSpawningComponent.Cast(component);

		int version;
		context.Read(version);

		bool readyForItem;
		bool itemTaken;
		CE_Item itemSpawned;
		int currentResetTime;
		UUID spawnerUUID;
		
		if (context.Read(readyForItem))
			spawningComp.SetReadyForItem(readyForItem);
		
		if (context.Read(itemTaken))
			spawningComp.SetHasSpawnedItemBeenTaken(itemTaken);
		
		if (context.Read(itemSpawned))
			spawningComp.SetItemSpawned(itemSpawned);
		
		if (context.Read(currentResetTime))
			spawningComp.SetCurrentSpawnerResetTime(currentResetTime);
		
		if (context.Read(spawnerUUID))
			spawningComp.SetSpawnerUUID(spawnerUUID);
		
		if (itemTaken == true)
		{
			CE_SpawnerTimingSystem spawnerTimingSystem = CE_SpawnerTimingSystem.GetByEntityWorld(owner);
			if (spawnerTimingSystem)
			{
				spawnerTimingSystem.RegisterSpawner(spawningComp);
			}
		}

		return true;
	}
}
