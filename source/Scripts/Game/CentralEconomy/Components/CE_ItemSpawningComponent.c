[ComponentEditorProps(category: "CentralEconomy/Components", description: "")]
class CE_ItemSpawningComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawningComponent : ScriptComponent
{
	/*
	[Attribute(ResourceName.Empty, UIWidgets.Object, "Item data config to be used", "conf", category: "Item Data")]
	ref array<ref ResourceName> m_sConfigs;
	*/
	
	[Attribute("", UIWidgets.Flags, desc: "Category of loot spawn", enums: ParamEnumArray.FromEnum(CE_ELootCategory), category: "Item Data")]
	CE_ELootCategory m_Categories;
	
	[Attribute("1800000", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the spawner to reset after spawned item was taken from it. Helps prevent loot camping.", category: "Item Data")] // default set to 1800000 seconds (30 minutes)
	int m_iSpawnerResetTime;
	
	protected CE_ELootUsage 								m_Usage; // gets set by the Usage Trigger Area Entity
	protected CE_ELootTier 								m_Tier; // gets set by the Tier Trigger Area Entity
		
	protected bool 										m_bHasItemSpawned 			= false;
	protected bool 										m_bHasItemDespawned 			= false;
	protected bool										m_bHasItemRestockEnded;
	
	protected CE_ItemData 								m_ItemSpawned;

	//------------------------------------------------------------------------------------------------
	protected void ConnectToLootSpawningSystem()
	{
		CE_ItemSpawningSystem updateSystem = CE_ItemSpawningSystem.GetInstance();
		
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromLootSpawningSystem()
	{
		CE_ItemSpawningSystem updateSystem = CE_ItemSpawningSystem.GetInstance();
		
		if (!updateSystem)
			return;

		updateSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnChildAdded(IEntity parent, IEntity child)
	{
		SetHasItemSpawned(true);
		
		m_bHasItemDespawned = false;
		
		m_bHasItemRestockEnded = false;
		
		GetGame().GetCallqueue().CallLater(DespawnItem, GetItemSpawned().m_iLifetime * 1000, false, child);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnChildRemoved(IEntity parent, IEntity child)
	{
		GetGame().GetCallqueue().CallLater(RemoveItemSpawned, 100, false, GetItemSpawned());
		GetGame().GetCallqueue().CallLater(idkwhattocallthis, 100, false);
		GetGame().GetCallqueue().CallLater(RestockReset, GetItemSpawned().m_iRestock * 1000, false, GetItemSpawned());
		
		CE_ItemSpawningSystem updateSystem = CE_ItemSpawningSystem.GetInstance();
		
		updateSystem.GetItemsNotRestocked().Insert(GetItemSpawned());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void idkwhattocallthis()
	{
		if (m_bHasItemDespawned)
		{
			SetHasItemSpawned(false);
		}
		else
		{
			GetGame().GetCallqueue().CallLater(SetHasItemSpawned, m_iSpawnerResetTime, false, false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RestockReset(CE_ItemData item)
	{
		m_bHasItemRestockEnded = true;
		
		CE_ItemSpawningSystem updateSystem = CE_ItemSpawningSystem.GetInstance();
		
		updateSystem.GetItemsNotRestocked().RemoveItem(item);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
		
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		ConnectToLootSpawningSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisconnectFromLootSpawningSystem();

		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasItemSpawned()
	{
		return m_bHasItemSpawned;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetHasItemSpawned(bool spawned)
	{
		m_bHasItemSpawned = spawned;
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ItemData GetItemSpawned()
	{
		return m_ItemSpawned;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetItemSpawned(notnull CE_ItemData item)
	{
		m_ItemSpawned = item;
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ELootTier GetSpawnerTier()
	{
		return m_Tier;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetSpawnerTier(CE_ELootTier tier)
	{
		m_Tier = tier;
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ELootUsage GetSpawnerUsage()
	{
		return m_Usage;
	}
		
	//------------------------------------------------------------------------------------------------
	void SetSpawnerUsage(CE_ELootUsage usage)
	{
		m_Usage = usage;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveItemSpawned(notnull CE_ItemData item)
	{
		CE_ItemSpawningSystem updateSystem = CE_ItemSpawningSystem.GetInstance();
		
		int index = updateSystem.GetSpawnedItems().Find(item);
		
		updateSystem.GetSpawnedItems().RemoveOrdered(index);
		
		GetGame().GetCallqueue().Remove(DespawnItem);
	}
	
	void DespawnItem(IEntity item)
	{
		if (item)
		{
			if (!m_bHasItemDespawned)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(item);
			
				m_bHasItemDespawned = true;
			}
		}
	}
}
