class CE_SearchableContainer
{
	protected CE_SearchableContainerComponent ContainerComponent;
	protected RplId ContainerRplID = RplId.Invalid();
	protected ref array<ref CE_Item> ItemsSpawned	= {};
	protected bool ReadyForItems;
	
	/*
	//------------------------------------------------------------------------------------------------
	void CE_SearchableContainer(CE_SearchableContainerComponent containerComponent, array<ref CE_Item> itemsSpawned, bool readyForItems)
	{
		ContainerComponent = containerComponent;
		ItemsSpawned = itemsSpawned;
		ReadyForItems = readyForItems;
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	//! 
	RplId GetContainerRplId()
	{
		if (ContainerComponent)
			return Replication.FindItemId(ContainerComponent.GetOwner());
		return RplId.Invalid();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the CE_SearchableContainerComponent corresponding to this CE_SearchableContainer
	CE_SearchableContainerComponent GetContainerComponent()
	{
		return ContainerComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets the CE_SearchableContainerComponent corresponding to this CE_SearchableContainer
	void SetContainerComponent(CE_SearchableContainerComponent containerComponent)
	{
		ContainerComponent = containerComponent;
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
		writer.WriteRplId(ContainerRplID);
		
		int itemCount = ItemsSpawned.Count();
		for (int i = 0; i < itemCount; ++i)
		{
			writer.Write(ItemsSpawned[i].GetItemData(), 4);
		}
		
		return true;
	}
	
	//! Property mem to snapshot extraction.
	// Extracts relevant properties from an instance of type T into snapshot. Opposite of Inject()
	static bool Extract(CE_SearchableContainer prop, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		array<ref CE_Item> itemsSpawned = prop.GetItemsSpawned();
		if (itemsSpawned)
		{
			int itemCount = itemsSpawned.Count();
			snapshot.SerializeInt(itemCount);
			
			for (int i = 0; i < itemCount; ++i)
			{
				snapshot.SerializeBytes(itemsSpawned[i].GetItemData(), 4);
				snapshot.SerializeBytes(itemsSpawned[i].GetTiers(), 1);
				snapshot.SerializeBytes(itemsSpawned[i].GetUsages(), 1);
				snapshot.SerializeBytes(itemsSpawned[i].GetCategory(), 1);
				snapshot.SerializeBytes(itemsSpawned[i].GetAvailableCount(), 4);
			}
		}
		
		snapshot.SerializeBytes(prop.GetContainerRplId(), 4);
		snapshot.SerializeBytes(prop.IsReadyForItems(), 4);
		
		return true;
	}

	//! Snapshot to property memory injection.
	// Injects relevant properties from snapshot into an instance of type T . Opposite of Extract()
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CE_SearchableContainer prop)
	{
		int itemCount;
		snapshot.SerializeInt(itemCount);
		
		if (prop.ItemsSpawned)
		{
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
				
				prop.ItemsSpawned.Insert(item);
			}
		}
		
		snapshot.SerializeBytes(prop.GetContainerRplId(), 4);
		snapshot.SerializeBytes(prop.IsReadyForItems(), 4);
		
		return true;
	}
	
	//! From snapshot to packet.
	// Takes snapshot and compresses it into packet. Opposite of Decode()
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		int itemCount;
		snapshot.SerializeBytes(itemCount, 4);
		packet.Serialize(itemCount, 32);
		
		for (int i = 0; i < itemCount; ++i)
		{
			snapshot.Serialize(packet, 11);
		}
		
		snapshot.Serialize(packet, 4);
		snapshot.Serialize(packet, 4);
	}

	//! From packet to snapshot.
	// Takes packet and decompresses it into snapshot. Opposite of Encode()
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		int itemCount;
		packet.Serialize(itemCount, 32);
		snapshot.SerializeBytes(itemCount, 4);
		
		for (int i = 0; i < itemCount; ++i)
		{
			snapshot.Serialize(packet, 11);
		}
		
		snapshot.Serialize(packet, 4);
		snapshot.Serialize(packet, 4);
		
		return true;
	}

	//! Snapshot to snapshot comparison.
	// Compares two snapshots to see whether they are the same or not
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 40);
		
		//return true;
	}

	//! Property mem to snapshot comparison.
	// Compares instance and a snapshot to see if any property has changed enough to require a new snapshot
	static bool PropCompare(CE_SearchableContainer prop, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.Compare(prop.GetContainerRplId(), 4)
			&& snapshot.Compare(prop.ItemsSpawned, 32)
			&& snapshot.CompareBool(prop.IsReadyForItems());
		
		//return true;
	}
}