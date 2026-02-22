class CE_Spawner
{
	protected UUID m_sSpawnerUUID;
	protected IEntity m_SpawnerEntity;
	protected bool m_bReadyForItem;
	protected UUID m_sItemUUID;
	
	/*
	void CE_Spawner()
	{
		//TESTING PURPOSES
		GetGame().GetCallqueue().CallLater(PrintSpawnerEntity, 5000, true);
	}
	
	void PrintSpawnerEntity()
	{
		//Print("MEOW Spawner Entity " + m_SpawnerEntity);
		Print("MEOW Spawner " + this + " " + m_sSpawnerUUID);
		//Print("MEOW Spawner Entity UUID " + m_sSpawnerUUID);
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! Returns the spawner's UUID
	UUID GetSpawnerUUID()
	{
		return m_sSpawnerUUID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the spawner's UUID
	void SetSpawnerUUID(UUID uuid)
	{
		m_sSpawnerUUID = uuid;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the spawner entity
	IEntity GetSpawnerEntity()
	{
		return m_SpawnerEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the spawner entity
	void SetSpawnerEntity(IEntity entity)
	{
		m_SpawnerEntity = entity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns if the CE_Spawner is ready for an item to be spawned
	bool IsReadyForItem()
	{
		return m_bReadyForItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the CE_Spawner is ready for an item to be spawned
	void SetReadyForItem(bool readyForItem)
	{
		m_bReadyForItem = readyForItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the spawned item's UUID
	UUID GetSpawnedItemUUID()
	{
		return m_sItemUUID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the spawned item's UUID
	void SetSpawnedItemUUID(UUID uuid)
	{
		m_sItemUUID = uuid;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_Item corresponding to this CE_Spawner, returns null if no item
	CE_Item GetItemSpawned()
	{
		if (!m_sItemUUID)
			return null;
		
		CE_ItemSpawningSystem spawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!spawningSystem)
			return null;
		
		return spawningSystem.FindItemByUUID(m_sItemUUID);
	}
}