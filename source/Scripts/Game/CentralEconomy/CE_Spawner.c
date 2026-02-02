class CE_Spawner
{
	protected UUID m_sSpawnerUUID;
	protected IEntity m_SpawnerEntity;
	protected ref CE_Item m_ItemSpawned;
	protected bool m_bReadyForItem;
	
	void CE_Spawner()
	{
		//TESTING PURPOSES
		GetGame().GetCallqueue().CallLater(PrintSpawnerEntity, 5000, true);
	}
	
	void PrintSpawnerEntity()
	{
		//Print("MEOW Spawner Entity " + m_SpawnerEntity);
		//Print("MEOW Spawner Entity UUID " + m_sSpawnerUUID);
		Print("MEOW Spawner " + this);
	}
	
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
	//! Returns the CE_Item corresponding to this CE_Spawner
	CE_Item GetItemSpawned()
	{
		return m_ItemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_Item corresponding to this CE_Spawner
	void SetItemSpawned(CE_Item itemSpawned)
	{
		m_ItemSpawned = itemSpawned;
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
}