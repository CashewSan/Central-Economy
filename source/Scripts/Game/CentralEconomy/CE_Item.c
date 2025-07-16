class CE_Item
{
	static const int DATA_SIZE_EXCLUDE_ITEMDATA = 16; // Tiers (4 bytes) + Usages (4 bytes) + Category (4 bytes) + AvailableCount (4 bytes)
	
	protected static CE_ItemData ItemData;
	protected CE_ELootTier Tiers;
	protected CE_ELootUsage Usages;
	protected CE_ELootCategory Category;
	protected int AvailableCount;
	
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
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool RplSave(ScriptBitWriter writer)
	{
		//ItemData.RplSave(writer);
		writer.Write(Tiers, 32);
		writer.Write(Usages, 32);
		writer.Write(Category, 32);
		writer.Write(AvailableCount, 32);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool RplLoad(ScriptBitReader reader)
	{
		//ItemData.RplLoad(reader);
		reader.Read(Tiers, 32);
		reader.Read(Usages, 32);
		reader.Read(Category, 32);
		reader.Read(AvailableCount, 32);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Extracts relevant properties from an instance of type T into snapshot. Opposite of Inject()
	static bool Extract(CE_Item prop, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		//prop.ItemData.Extract(prop.ItemData, ctx, snapshot);
		snapshot.SerializeBytes(prop.Tiers, 4);
		snapshot.SerializeBytes(prop.Usages, 4);
		snapshot.SerializeBytes(prop.Category, 4);
		snapshot.SerializeBytes(prop.AvailableCount, 4);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Injects relevant properties from snapshot into an instance of type T . Opposite of Extract()
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CE_Item prop)
	{
		//prop.ItemData.Inject(snapshot, ctx, prop.ItemData);
		snapshot.SerializeBytes(prop.Tiers, 4);
		snapshot.SerializeBytes(prop.Usages, 4);
		snapshot.SerializeBytes(prop.Category, 4);
		snapshot.SerializeBytes(prop.AvailableCount, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes snapshot and compresses it into packet. Opposite of Decode()
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		//ItemData.Encode(snapshot, ctx, packet);
		snapshot.Serialize(packet, CE_Item.DATA_SIZE_EXCLUDE_ITEMDATA);
	}

	//------------------------------------------------------------------------------------------------
	//! Takes packet and decompresses it into snapshot. Opposite of Encode()
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return /*ItemData.Decode(packet, ctx, snapshot)
			&&*/ snapshot.Serialize(packet, CE_Item.DATA_SIZE_EXCLUDE_ITEMDATA);
	}

	//------------------------------------------------------------------------------------------------
	//! Compares two snapshots to see whether they are the same or not
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return /*ItemData.SnapCompare(lhs, rhs, ctx)
			&&*/ lhs.CompareSnapshots(rhs, CE_Item.DATA_SIZE_EXCLUDE_ITEMDATA);
	}

	//------------------------------------------------------------------------------------------------
	//! Compares instance and a snapshot to see if any property has changed enough to require a new snapshot
	static bool PropCompare(CE_Item prop, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return /*prop.ItemData.PropCompare(prop.ItemData, snapshot, ctx)
			&& */snapshot.Compare(prop.Tiers, 4)
			&& snapshot.Compare(prop.Usages, 4)
			&& snapshot.Compare(prop.Category, 4)
			&& snapshot.Compare(prop.AvailableCount, 4);
	}
}