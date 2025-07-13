class CE_Item
{
	protected CE_ItemData ItemData;
	protected CE_ELootTier Tiers;
	protected CE_ELootUsage Usages;
	protected CE_ELootCategory Category;
	protected int AvailableCount;
	
	//------------------------------------------------------------------------------------------------
	void CE_Item(CE_ItemData itemData, CE_ELootTier tiers, CE_ELootUsage usages, CE_ELootCategory category, int availableCount)
	{
		ItemData = itemData;
		Tiers = tiers;
		Usages = usages;
		Category = category;
		AvailableCount = availableCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ItemData corresponding to this CE_Item
	CE_ItemData GetItemData()
	{
		return ItemData;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ItemData corresponding to this CE_Item
	void SetItemData(CE_ItemData itemData)
	{
		ItemData = itemData;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ELootTier(s) corresponding to this CE_Item
	CE_ELootTier GetTiers()
	{
		return Tiers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootTier(s) corresponding to this CE_Item
	void SetTiers(CE_ELootTier tiers)
	{
		Tiers = tiers;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ELootUsage(s) corresponding to this CE_Item
	CE_ELootUsage GetUsages()
	{
		return Usages;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootUsage(s) corresponding to this CE_Item
	void SetUsages(CE_ELootUsage usages)
	{
		Usages = usages;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_ELootCategory corresponding to this CE_Item
	CE_ELootCategory GetCategory()
	{
		return Category;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_ELootCategory corresponding to this CE_Item
	void SetCategory(CE_ELootCategory category)
	{
		Category = category;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the available item count corresponding to this CE_Item
	int GetAvailableCount()
	{
		return AvailableCount;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the available item count corresponding to this CE_Item
	void SetAvailableCount(int availableCount)
	{
		AvailableCount = availableCount;
	}
}