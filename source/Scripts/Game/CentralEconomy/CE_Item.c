class CE_Item
{
	protected ref CE_ItemData m_ItemData;
	protected CE_ELootTier m_Tiers;
	protected CE_ELootUsage m_Usages;
	protected CE_ELootCategory m_Category;
	protected int m_iAvailableCount;
	
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
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ItemData corresponding to this CE_Item
	CE_ItemData GetItemData()
	{
		return m_ItemData;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ItemData corresponding to this CE_Item
	void SetItemData(CE_ItemData itemData)
	{
		m_ItemData = itemData;
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
}