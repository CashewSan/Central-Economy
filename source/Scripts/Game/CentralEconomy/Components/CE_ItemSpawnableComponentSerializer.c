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
			Print("MEOW WRITING DATA");
			
			context.WriteValue("spawner", spawnableComp.GetSpawner());
			context.WriteValue("item", spawnableComp.GetItem());
			context.WriteValue("totalRestockTime", spawnableComp.GetTotalRestockTime());
			context.WriteValue("currentRestockTime", spawnableComp.GetCurrentRestockTime());
			context.WriteValue("totalLifetime", spawnableComp.GetTotalLifetime());
			context.WriteValue("currentLifetime", spawnableComp.GetCurrentLifetime());
			context.WriteValue("restockEnded", spawnableComp.HasRestockEnded());
			context.WriteValue("itemTaken", spawnableComp.WasItemTaken());
			context.WriteValue("wasDeposited", spawnableComp.WasDepositedByAction());
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
			Print("MEOW READING DATA");
			
			CE_Spawner spawner;
			CE_Item item;
			int totalRestockTime;
			int currentRestockTime;
			int totalLifetime;
			int currentLifetime;
			bool restockEnded;
			bool itemTaken;
			bool wasDeposited;
			
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
			
			if (context.Read(item))
				spawnableComp.SetItem(item);
			
			if (context.Read(spawner))
			{
				spawnableComp.SetSpawner(spawner);
				
				if (spawner)
				{
					IEntity spawnerEntity = spawner.GetSpawnerEntity();
					if (spawnerEntity)
					{
						CE_ItemSpawningComponent spawnerComp = CE_ItemSpawningComponent.Cast(spawnerEntity.FindComponent(CE_ItemSpawningComponent));
						if (spawnerComp)
						{
							spawnerComp.SetEntitySpawned(owner);
						}
						else
							Print("[CENTRALECONOMY::CE_ItemSpawnableComponentSerializer] - COULD NOT SET ENTITY SPAWNED. NO CE_ItemSpawningComponent", LogLevel.ERROR);
					}
					else
						Print("[CENTRALECONOMY::CE_ItemSpawnableComponentSerializer] - COULD NOT SET ENTITY SPAWNED. NO Spawner Entity", LogLevel.ERROR);
				}
				else
					Print("[CENTRALECONOMY::CE_ItemSpawnableComponentSerializer] - COULD NOT SET ENTITY SPAWNED. NO CE_Spawner", LogLevel.ERROR);
			}
			
			CE_ItemSpawnableSystem spawnableSystem = CE_ItemSpawnableSystem.GetByEntityWorld(owner);
			if (spawnableSystem)
			{
				spawnableSystem.Register(spawnableComp);
			}
		}

		return true;
	}
}
