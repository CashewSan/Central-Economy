/*
class CE_SearchableContainer
{
	static const int DATA_SIZE_EXCLUDE_ITEMSSPAWNED = 5; // ContainerRplId (4 bytes) + ReadyForItems (We'll send it as 1 byte) 
	
	protected CE_SearchableContainerComponent m_ContainerComponent; // not replicated
	protected RplId ContainerRplId = RplId.Invalid();
	protected ref array<ref CE_Item> ItemsSpawned = {};
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
	//! 
	CE_SearchableContainerComponent GetContainerComponent()
	{
		return m_ContainerComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	void SetContainerComponent(CE_SearchableContainerComponent component)
	{
		m_ContainerComponent = component;
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
		writer.Write(ReadyForItems, 1);
		
		int itemCount = ItemsSpawned.Count();
		writer.Write(itemCount, 32);
		
		for (int i = 0; i < itemCount; ++i)
		{
			ItemsSpawned[i].RplSave(writer);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! 
	bool RplLoad(ScriptBitReader reader)
	{
		reader.Read(ContainerRplId, 32);
		reader.Read(ReadyForItems, 1);
		
		int itemCount = ItemsSpawned.Count();
		reader.Read(itemCount, 32);
		
		for (int i = 0; i < itemCount; ++i)
		{
			ItemsSpawned[i].RplLoad(reader);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Extracts relevant properties from an instance of type T into snapshot. Opposite of Inject()
	static bool Extract(CE_SearchableContainer prop, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		if (!prop)
			return false;
		
		snapshot.SerializeBytes(prop.ContainerRplId, 4);
		snapshot.SerializeBytes(prop.ReadyForItems, 1);
		
		int itemCount = prop.ItemsSpawned.Count();
		snapshot.SerializeBytes(itemCount, 4);
		
		for (int i = 0; i < itemCount; ++i)
		{
			prop.ItemsSpawned[i].Extract(prop.ItemsSpawned[i], ctx, snapshot);
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Injects relevant properties from snapshot into an instance of type T . Opposite of Extract()
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, CE_SearchableContainer prop)
	{
		if (!prop)
			return false;
		
		snapshot.SerializeBytes(prop.ContainerRplId, 4);
		snapshot.SerializeBytes(prop.ReadyForItems, 1);
		
		int itemCount;
		snapshot.SerializeBytes(itemCount, 4);
		
		prop.ItemsSpawned.Clear();
		
		CE_Item item;
		for (int i = 0; i < itemCount; ++i)
		{
			item = new CE_Item();
			
			item.Inject(snapshot, ctx, item);
			
			prop.ItemsSpawned.Insert(item);
		}
		
		prop.SetContainerComponent(CE_SearchableContainerComponent.Cast(Replication.FindItem(prop.ContainerRplId)));
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Takes snapshot and compresses it into packet. Opposite of Decode()
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.Serialize(packet, CE_SearchableContainer.DATA_SIZE_EXCLUDE_ITEMSSPAWNED);
		
		int itemCount;
		snapshot.SerializeBytes(itemCount, 4);
		packet.Serialize(itemCount, 32);

		for (int i = 0; i < itemCount; ++i)
		{
			ItemsSpawned[i].Encode(snapshot, ctx, packet);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Takes packet and decompresses it into snapshot. Opposite of Encode()
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.Serialize(packet, CE_SearchableContainer.DATA_SIZE_EXCLUDE_ITEMSSPAWNED);
		
		int itemCount;
		packet.Serialize(itemCount, 32);
		snapshot.SerializeBytes(itemCount, 4);

		for (int i = 0; i < itemCount; ++i)
		{
			ItemsSpawned[i].Decode(packet, ctx, snapshot);
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Compares two snapshots to see whether they are the same or not
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx)
	{
		lhs.CompareSnapshots(rhs, CE_SearchableContainer.DATA_SIZE_EXCLUDE_ITEMSSPAWNED);
		
		int itemCount = ItemsSpawned.Count();
		
		for (int i = 0; i < itemCount; ++i)
		{
			ItemsSpawned[i].SnapCompare(lhs, rhs, ctx);
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Compares instance and a snapshot to see if any property has changed enough to require a new snapshot
	static bool PropCompare(CE_SearchableContainer prop, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		if (!prop)
			return false;
		
		snapshot.Compare(prop.ContainerRplId, 4);
		snapshot.Compare(prop.ReadyForItems, 1);
		
		int itemCount = prop.ItemsSpawned.Count();
		
		for (int i = 0; i < itemCount; ++i)
		{
			prop.ItemsSpawned[i].PropCompare(prop.ItemsSpawned[i], snapshot, ctx);
		}
		
		return true;
	}
}
*/