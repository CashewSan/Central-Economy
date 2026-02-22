class CE_Item
{
	//protected ref CE_ItemData m_ItemData;
	protected string m_sItemDataName;
	protected CE_ELootTier m_Tiers;
	protected CE_ELootUsage m_Usages;
	protected CE_ELootCategory m_Category;
	protected int m_iAvailableCount;
	protected UUID m_sItemUUID;
	
	/*
	void CE_Item()
	{
		//TESTING PURPOSES
		GetGame().GetCallqueue().CallLater(PrintItemEntity, 5000, true);
	}
	
	void PrintItemEntity()
	{
		//Print("MEOW Spawner Entity " + m_SpawnerEntity);
		//Print("MEOW Spawner Entity UUID " + m_sSpawnerUUID);
		Print("MEOW Item " + this);
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ItemData name corresponding to this CE_Item
	string GetItemDataName()
	{
		return m_sItemDataName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ItemData name corresponding to this CE_Item
	void SetItemDataName(string itemDataName)
	{
		m_sItemDataName = itemDataName;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ELootTier(s) corresponding to this CE_Item
	CE_ELootTier GetTiers()
	{
		return m_Tiers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootTier(s) corresponding to this CE_Item
	void SetTiers(CE_ELootTier tiers)
	{
		m_Tiers = tiers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ELootUsage(s) corresponding to this CE_Item
	CE_ELootUsage GetUsages()
	{
		return m_Usages;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootUsage(s) corresponding to this CE_Item
	void SetUsages(CE_ELootUsage usages)
	{
		m_Usages = usages;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ELootCategory corresponding to this CE_Item
	CE_ELootCategory GetCategory()
	{
		return m_Category;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootCategory corresponding to this CE_Item
	void SetCategory(CE_ELootCategory category)
	{
		m_Category = category;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the available item count corresponding to this CE_Item
	int GetAvailableCount()
	{
		return m_iAvailableCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the available item count corresponding to this CE_Item
	void SetAvailableCount(int availableCount)
	{
		m_iAvailableCount = availableCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the item's UUID
	UUID GetItemUUID()
	{
		return m_sItemUUID;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the item's UUID
	void SetItemUUID(UUID uuid)
	{
		m_sItemUUID = uuid;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_Item corresponding to this CE_Spawner, returns null if no item
	CE_ItemData GetItemData()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sItemDataName))
			return null;
		
		CE_ItemSpawningSystem spawningSystem = CE_ItemSpawningSystem.GetInstance();
		if (!spawningSystem)
			return null;
		
		return spawningSystem.FindItemDataByName(m_sItemDataName);
	}
}