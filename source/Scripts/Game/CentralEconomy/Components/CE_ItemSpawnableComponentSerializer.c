class CE_ItemSpawnableComponentSerializer : ScriptedComponentSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return CE_ItemSpawnableComponent;
	}

	//------------------------------------------------------------------------------------------------
	override protected ESerializeResult Serialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationSaveContext context)
	{
		const CE_ItemSpawnableComponent spawnableComp = CE_ItemSpawnableComponent.Cast(component);

		context.WriteValue("version", 1);
		context.WriteValue("spawnedBySystem", spawnableComp.WasSpawnedBySystem());
		
		// only write data if the item was spawned by the CE system
		if (spawnableComp.WasSpawnedBySystem())
		{
			context.WriteValue("totalRestockTime", spawnableComp.GetTotalRestockTime());
			context.WriteValue("currentRestockTime", spawnableComp.GetCurrentRestockTime());
			context.WriteValue("totalLifetime", spawnableComp.GetTotalLifetime());
			context.WriteValue("currentLifetime", spawnableComp.GetCurrentLifetime());
			context.WriteValue("restockEnded", spawnableComp.HasRestockEnded());
			context.WriteValue("itemTaken", spawnableComp.WasItemTaken());
			context.WriteValue("wasDeposited", spawnableComp.WasDepositedByAction());
			context.WriteValue("itemUUID", spawnableComp.GetSpawnedItemUUID());
			context.WriteValue("spawnerUUID", spawnableComp.GetSpawnerUUID());
		}
		
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool Deserialize(notnull IEntity owner, notnull GenericComponent component, notnull BaseSerializationLoadContext context)
	{
		CE_ItemSpawnableComponent spawnableComp = CE_ItemSpawnableComponent.Cast(component);

		int version;
		context.Read(version);

		bool spawnedBySystem;
		if (context.Read(spawnedBySystem))
			spawnableComp.SetWasSpawnedBySystem(spawnedBySystem);
		
		// only read data if the item was spawned by the CE system
		if (spawnedBySystem == true)
		{
			CE_ItemSpawningSystem spawningSystem = CE_ItemSpawningSystem.GetByEntityWorld(owner);
			
			int totalRestockTime;
			int currentRestockTime;
			int totalLifetime;
			int currentLifetime;
			bool restockEnded;
			bool itemTaken;
			bool wasDeposited;
			UUID itemUUID;
			UUID spawnerUUID;
			
			if (context.Read(totalRestockTime))
				spawnableComp.SetTotalRestockTime(totalRestockTime);
			
			if (context.Read(currentRestockTime))
				spawnableComp.SetCurrentRestockTime(currentRestockTime);
			
			if (context.Read(totalLifetime))
				spawnableComp.SetTotalLifetime(totalLifetime);
			
			if (context.Read(currentLifetime))
				spawnableComp.SetCurrentLifetime(currentLifetime);
			
			if (context.Read(restockEnded))
				spawnableComp.SetHasRestockEnded(restockEnded);
			
			if (context.Read(itemTaken))
				spawnableComp.SetWasItemTaken(itemTaken);
			
			if (context.Read(wasDeposited))
				spawnableComp.SetWasDepositedByAction(wasDeposited);
			
			if (context.Read(itemUUID))
			{
				spawnableComp.SetSpawnedItemUUID(itemUUID);
				
				if (!itemUUID.IsNull() && spawningSystem)
				{
					spawnableComp.SetSpawnedItem(spawningSystem.FindItemByUUID(itemUUID));
				}
			}
			
			if (context.Read(spawnerUUID))
			{
				spawnableComp.SetSpawnerUUID(spawnerUUID);
				
				if (!spawnerUUID.IsNull() && spawningSystem)
				{
					spawnableComp.SetSpawner(spawningSystem.FindSpawnerByUUID(spawnerUUID));
					
					CE_ItemSpawningComponent spawningComp = spawningSystem.FindSpawningComponentByUUID(spawnerUUID);
					if (spawningComp)
						spawningComp.SetEntitySpawned(owner);
				}
			}
			
			CE_ItemSpawnableSystem spawnableSystem = CE_ItemSpawnableSystem.GetByEntityWorld(owner);
			if (spawnableSystem)
			{
				spawnableSystem.Register(spawnableComp);
			}
			
			if (!itemTaken && !spawnerUUID.IsNull() && !itemUUID.IsNull() && spawningSystem)
			{
				//spawningSystem.RegisterSpawnable(spawnableComp);
				CE_Spawner spawner = spawningSystem.FindSpawnerByUUID(spawnerUUID);
				CE_ItemSpawningComponent spawningComp = spawningSystem.FindSpawningComponentByUUID(spawnerUUID);
				CE_Item item = spawningSystem.FindItemByUUID(itemUUID);
				
				if (spawningComp && spawner && item)
				{
					spawningComp.GetItemSpawnedInvoker().Invoke(owner, item, spawner);
				}
			}
		}

		return true;
	}
}