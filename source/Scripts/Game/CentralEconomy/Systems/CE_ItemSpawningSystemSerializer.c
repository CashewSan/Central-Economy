class CE_PersistentSpawner
{
	UUID m_sSpawnerUUID;
	UUID m_sSpawnedItemUUID;
	bool m_bReadyForItem;
}

class CE_PersistentItem
{
	string m_sItemDataName;
	CE_ELootTier m_Tiers;
	CE_ELootUsage m_Usages;
	CE_ELootCategory m_Category;
	int m_iAvailableCount;
	UUID m_sItemUUID;
}

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
		
		system.SetPersistenceDataAvailability(true);
		
		// CE_Items
		array <ref CE_PersistentItem> persistentItems();
		array <ref CE_Item> items = system.GetItems();
		foreach (CE_Item item : items)
		{
			CE_PersistentItem persistentItem();
			persistentItem.m_sItemDataName = item.GetItemDataName();
			persistentItem.m_Tiers = item.GetTiers();
			persistentItem.m_Usages = item.GetUsages();
			persistentItem.m_Category = item.GetCategory();
			persistentItem.m_iAvailableCount = item.GetAvailableCount();
			persistentItem.m_sItemUUID = item.GetItemUUID();
			persistentItems.Insert(persistentItem);
		}
		
		// CE_Spawners
		array <ref CE_PersistentSpawner> persistentSpawners();
		array <ref CE_Spawner> spawners = system.GetSpawners();
		foreach (CE_Spawner spawner : spawners)
		{
			CE_PersistentSpawner persistentSpawner();
			persistentSpawner.m_sSpawnerUUID = spawner.GetSpawnerUUID();
			persistentSpawner.m_sSpawnedItemUUID = spawner.GetSpawnedItemUUID();
			persistentSpawner.m_bReadyForItem = spawner.IsReadyForItem();
			persistentSpawners.Insert(persistentSpawner);
		}
		
		context.WriteValue("version", 1);
		context.WriteValue("availablePersistence", system.ExistingPersistenceDataAvailability());
		context.WriteValue("initialSpawnRan", system.HasInitialSpawnRan());
		context.WriteValue("itemCount", system.GetItemCount());
		
		const bool prev = context.EnableTypeDiscriminator(false);
		context.WriteValue("items", persistentItems);
		context.EnableTypeDiscriminator(prev);
		
		const bool prevv = context.EnableTypeDiscriminator(false);
		context.WriteValue("spawners", persistentSpawners);
		context.EnableTypeDiscriminator(prevv);
		
		return ESerializeResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override bool Deserialize(notnull Managed instance, notnull BaseSerializationLoadContext context)
	{
		CE_ItemSpawningSystem system = CE_ItemSpawningSystem.GetInstance();
		if (!system)
			return true;

		int version;
		context.Read(version);
		
		bool availablePersistence;
		if (context.Read(availablePersistence))
			system.SetPersistenceDataAvailability(availablePersistence);
		
		bool initialSpawnRan;
		if (context.Read(initialSpawnRan))
			system.SetHasInitialSpawnRan(initialSpawnRan);

		int itemCount;
		if (context.Read(itemCount))
			system.SetItemCount(itemCount);
		
		// CE_Items
		array<ref CE_PersistentItem> items;
		const bool prev = context.EnableTypeDiscriminator(false);
		context.Read(items);
		context.EnableTypeDiscriminator(prev);	
		if (items)
		{
			array<ref CE_Item> systemItems = system.GetItems();
			
			foreach (CE_PersistentItem persistentItem : items)
			{
				CE_Item item();
				item.SetItemDataName(persistentItem.m_sItemDataName);
				item.SetTiers(persistentItem.m_Tiers);
				item.SetUsages(persistentItem.m_Usages);
				item.SetCategory(persistentItem.m_Category);
				item.SetAvailableCount(persistentItem.m_iAvailableCount);
				item.SetItemUUID(persistentItem.m_sItemUUID);
				
				if (systemItems)
					systemItems.Insert(item);
			}
		}
		
		// CE_Spawners
		array<ref CE_PersistentSpawner> spawners;
		const bool prevv = context.EnableTypeDiscriminator(false);
		context.Read(spawners);
		context.EnableTypeDiscriminator(prevv);
		if (spawners)
		{
			int count = 0;
			
			array<ref CE_Spawner> systemSpawners = system.GetSpawners();
			
			foreach (CE_PersistentSpawner persistentSpawner : spawners)
			{
				CE_Spawner spawner();
				spawner.SetSpawnerUUID(persistentSpawner.m_sSpawnerUUID);
				spawner.SetSpawnedItemUUID(persistentSpawner.m_sSpawnedItemUUID);
				spawner.SetReadyForItem(persistentSpawner.m_bReadyForItem);
				
				if (systemSpawners)
				{
					systemSpawners.Insert(spawner);
					count++;
				}
			}
		}
		
		system.SetPersistenceFinished(true);
		
		return true;
	}
}