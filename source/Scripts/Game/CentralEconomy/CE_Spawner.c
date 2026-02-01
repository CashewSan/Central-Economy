class CE_Spawner
{
	//protected CE_ItemSpawningComponent m_SpawningComponent;
	protected IEntity m_SpawnerEntity;
	protected ref CE_Item m_ItemSpawned;
	protected bool m_ReadyForItem;
	
	/*
	//------------------------------------------------------------------------------------------------
	void CE_Spawner(CE_ItemSpawningComponent spawningComponent, CE_Item itemSpawned, bool readyForItem)
	{
		m_SpawningComponent = spawningComponent;
		m_ItemSpawned = itemSpawned;
		m_ReadyForItem = readyForItem;
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! 
	IEntity GetSpawnerEntity()
	{
		return m_SpawnerEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	void SetSpawnerEntity(IEntity entity)
	{
		m_SpawnerEntity = entity;
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ItemSpawningComponent corresponding to this CE_Spawner
	CE_ItemSpawningComponent GetSpawningComponent()
	{
		return m_SpawningComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ItemSpawningComponent corresponding to this CE_Spawner
	void SetSpawningComponent(CE_ItemSpawningComponent spawningComponent)
	{
		m_SpawningComponent = spawningComponent;
	}
	*/
	
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
		return m_ReadyForItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the CE_Spawner is ready for an item to be spawned
	void SetReadyForItem(bool readyForItem)
	{
		m_ReadyForItem = readyForItem;
	}
}