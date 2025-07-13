class CE_Spawner
{
	protected CE_ItemSpawningComponent SpawningComponent;
	protected CE_Item ItemSpawned;
	protected bool ReadyForItem;
	
	//------------------------------------------------------------------------------------------------
	void CE_Spawner(CE_ItemSpawningComponent spawningComponent, CE_Item itemSpawned, bool readyForItem)
	{
		SpawningComponent = spawningComponent;
		ItemSpawned = itemSpawned;
		ReadyForItem = readyForItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ItemSpawningComponent corresponding to this CE_Spawner
	CE_ItemSpawningComponent GetSpawningComponent()
	{
		return SpawningComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ItemSpawningComponent corresponding to this CE_Spawner
	void SetSpawningComponent(CE_ItemSpawningComponent spawningComponent)
	{
		SpawningComponent = spawningComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_Item corresponding to this CE_Spawner
	CE_Item GetItemSpawned()
	{
		return ItemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_Item corresponding to this CE_Spawner
	void SetItemSpawned(CE_Item itemSpawned)
	{
		ItemSpawned = itemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns if the CE_Spawner is ready for an item to be spawned
	bool IsReadyForItem()
	{
		return ReadyForItem;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the CE_Spawner is ready for an item to be spawned
	void SetReadyForItem(bool readyForItem)
	{
		ReadyForItem = readyForItem;
	}
}