class CE_ItemSpawningSystemData : PersistentState
{
}

class CE_ItemSpawningSystemSerializer : ScriptedStateSerializer
{
	//------------------------------------------------------------------------------------------------
	override static typename GetTargetType()
	{
		return CE_ItemSpawningSystemData;
	}

	//------------------------------------------------------------------------------------------------
	override ESerializeResult Serialize(notnull Managed instance, notnull BaseSerializationSaveContext context)
	{
		CE_ItemSpawningSystem system = CE_ItemSpawningSystem.GetInstance();
		if (!system)
			return ESerializeResult.DEFAULT;

		context.WriteValue("version", 1);
		context.WriteValue("itemCount", system.GetItemCount());
		
		array<ref CE_Item> items();
		array<ref CE_Item> systemItems = system.GetItems();
		foreach (CE_Item item : systemItems)
		{
			items.Insert(item);
		}
		
		const bool prev = context.EnableTypeDiscriminator(false);
		context.WriteValue("items", items);
		context.EnableTypeDiscriminator(prev);
		
		array<ref CE_Spawner> spawners();
		array<ref CE_Spawner> systemSpawners = system.GetSpawners();
		foreach (CE_Spawner spawner : systemSpawners)
		{
			spawners.Insert(spawner);
		}
		
		const bool prevv = context.EnableTypeDiscriminator(false);
		context.WriteValue("spawners", spawners);
		context.EnableTypeDiscriminator(prevv);
		
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		CE_ItemSpawningSystem system = CE_ItemSpawningSystem.GetInstance();
		if (!system)
			return false;

		int version;
		context.Read(version);

		int itemCount;
		
		if (context.Read(itemCount))
			system.SetItemCount(itemCount);
		
		// CE_Items
		array<ref CE_Item> items();
		const bool prev = context.EnableTypeDiscriminator(false);
		context.Read(items);
		context.EnableTypeDiscriminator(prev);
		if (items)
		{
			array<ref CE_Item> systemItems = system.GetItems();
			
			foreach (CE_Item item : items)
			{
				if (systemItems)
					systemItems.Insert(item);
			}
		}
		
		// CE_Spawners
		array<ref CE_Spawner> spawners();
		const bool prevv = context.EnableTypeDiscriminator(false);
		context.Read(spawners);
		context.EnableTypeDiscriminator(prevv);
		if (spawners)
		{
			array<ref CE_Spawner> systemSpawners = system.GetSpawners();
			
			foreach (CE_Spawner spawner : spawners)
			{
				if (systemSpawners)
					systemSpawners.Insert(spawner);
			}
		}

		return true;
	}
}