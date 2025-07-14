class CE_SearchableContainer
{
	static const int DATA_SIZE = 5; // ContainerRplId (4 bytes) + ReadyForItems (We'll send it as 1 byte)
	
	protected RplId ContainerRplId = RplId.Invalid();
	protected ref array<ref CE_Item> ItemsSpawned	= {};
	protected bool ReadyForItems = true;
	
	//------------------------------------------------------------------------------------------------
	//! 
	CE_SearchableContainerComponent GetContainerComponentFromRplId(RplId rplId)
	{
		return CE_SearchableContainerComponent.Cast(Replication.FindItem(rplId));
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	RplId GetContainerRplId()
	{
		return ContainerRplId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	void SetContainerRplId(RplId rplId)
	{
		ContainerRplId = rplId;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_Item(s) spawned corresponding to this CE_SearchableContainer
	array<ref CE_Item> GetItemsSpawned()
	{
		return ItemsSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns if the CE_SearchableContainer is ready for items to be spawned
	bool IsReadyForItems()
	{
		return ReadyForItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets if the CE_SearchableContainer is ready for items to be spawned
	void SetReadyForItems(bool readyForItems)
	{
		ReadyForItems = readyForItems;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool RplSave(ScriptBitWriter writer)
	{
		writer.Write(ContainerRplId, 32); // 4 bytes * 8 = 32 bits.
		
		/*
		int itemCount = ItemsSpawned.Count();
		writer.WriteInt(itemCount);
		
		for (int i = 0; i < itemCount; ++i)
		{
			writer.Write(ItemsSpawned[i].GetItemData(), 4);
			writer.Write(ItemsSpawned[i].GetTiers(), 1);
			writer.Write(ItemsSpawned[i].GetUsages(), 1);
			writer.Write(ItemsSpawned[i].GetCategory(), 1);
			writer.Write(ItemsSpawned[i].GetAvailableCount(), 4);
		}
		*/
		
		writer.Write(ReadyForItems, 1);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool RplLoad(ScriptBitReader reader)
	{
		reader.Read(ContainerRplId, 32);
		
		/*
		int itemCount = ItemsSpawned.Count();
		reader.ReadInt(itemCount);
		
		for (int i = 0; i < itemCount; ++i)
		{
			reader.Read(ItemsSpawned[i].GetItemData(), 4);
			reader.Read(ItemsSpawned[i].GetTiers(), 1);
			reader.Read(ItemsSpawned[i].GetUsages(), 1);
			reader.Read(ItemsSpawned[i].GetCategory(), 1);
			reader.Read(ItemsSpawned[i].GetAvailableCount(), 4);
		}
		*/
		
		reader.Read(ReadyForItems, 1);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Extracts relevant properties from an instance of type T into snapshot. Opposite of Inject()
	static bool Extract(CE_SearchableContainer prop, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeBytes(prop.ContainerRplId, 4);
		
		/*
		int itemCount = prop.ItemsSpawned.Count();
		snapshot.SerializeInt(itemCount);
		
		for (int i = 0; i < itemCount; ++i)
		{
			snapshot.SerializeBytes(prop.ItemsSpawned[i].GetItemData(), 4);
			snapshot.SerializeBytes(prop.ItemsSpawned[i].GetTiers(), 1);
			snapshot.SerializeBytes(prop.ItemsSpawned[i].GetUsages(), 1);
			snapshot.SerializeBytes(prop.ItemsSpawned[i].GetCategory(), 1);
			snapshot.SerializeBytes(prop.ItemsSpawned[i].GetAvailableCount(), 4);
		}
		*/
		
		snapshot.SerializeBytes(prop.ReadyForItems, 1);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Injects relevant properties from snapshot into an instance of type T . Opposite of Extract()
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CE_SearchableContainer prop)
	{
		snapshot.SerializeBytes(prop.ContainerRplId, 4);
		
		/*
		int itemCount;
		snapshot.SerializeInt(itemCount);
		
		prop.ItemsSpawned.Clear();
		
		CE_Item item;
		for (int i = 0; i < itemCount; ++i)
		{
			CE_ItemData itemData;
			snapshot.SerializeBytes(itemData, 4);
			
			CE_ELootTier tier;
			snapshot.SerializeBytes(tier, 1);
			
			CE_ELootUsage usage;
			snapshot.SerializeBytes(usage, 1);
			
			CE_ELootCategory category;
			snapshot.SerializeBytes(category, 1);
			
			int availableCount;
			snapshot.SerializeBytes(availableCount, 4);
			
			item = new CE_Item(itemData, tier, usage, category, availableCount);
			
			/*
			item.SetItemData(itemData);
			item.SetTiers(tier);
			item.SetUsages(usage);
			item.SetCategory(category);
			item.SetAvailableCount(availableCount);
			*/
			/*
			prop.ItemsSpawned.Insert(item);
		}
		*/
		
		snapshot.SerializeBytes(prop.ReadyForItems, 1);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes snapshot and compresses it into packet. Opposite of Decode()
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.Serialize(packet, CE_SearchableContainer.DATA_SIZE);
	}

	//------------------------------------------------------------------------------------------------
	//! Takes packet and decompresses it into snapshot. Opposite of Encode()
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		return snapshot.Serialize(packet, CE_SearchableContainer.DATA_SIZE);
	}

	//------------------------------------------------------------------------------------------------
	//! Compares two snapshots to see whether they are the same or not
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, CE_SearchableContainer.DATA_SIZE);
	}

	//------------------------------------------------------------------------------------------------
	//! Compares instance and a snapshot to see if any property has changed enough to require a new snapshot
	static bool PropCompare(CE_SearchableContainer prop, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.Compare(prop.ContainerRplId, 4)
			//&& snapshot.Compare(prop.ItemsSpawned, 24) // itemCount = 4, GetItemData() = 4, GetItemTiers() = 4, GetItemUsages() = 4, GetItemCategory() = 4, GetAvailableCount() = 4
			&& snapshot.Compare(prop.ReadyForItems, 1);
	}
}