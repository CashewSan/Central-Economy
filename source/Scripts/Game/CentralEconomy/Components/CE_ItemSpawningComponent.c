[ComponentEditorProps(category: "CentralEconomy/Components", description: "")]
class CE_ItemSpawningComponentClass : ScriptComponentClass
{
}

class CE_ItemSpawningComponent : ScriptComponent
{
	[Attribute(ResourceName.Empty, UIWidgets.Object, "Item data config to be used (If set, universal config through CE_WorldValidationComponent will be ignored)", "conf", category: "Item Data")]
	ref CE_ItemDataConfig m_ItemDataConfig;
	
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, desc: "Prefab to be spawned (If set, Item Data Config will be ignored)", "et", category: "Item Data")]
    ResourceName m_sPrefab;
	
	[Attribute("", UIWidgets.Flags, desc: "Category of loot spawn", enums: ParamEnumArray.FromEnum(CE_ELootCategory), category: "Item Data")]
	CE_ELootCategory m_Categories;
	
	[Attribute("1800000", UIWidgets.EditBox, desc: "Time (in seconds) it takes for the spawner to reset after spawned item was taken from it. Helps prevent loot camping.", category: "Item Data")] // default set to 1800000 seconds (30 minutes)
	int m_iSpawnerResetTime;
	
	protected CE_ELootUsage 								m_Usage; 																	// gets set by the Usage Trigger Area Entity
	protected CE_ELootTier 								m_Tier; 																		// gets set by the Tier Trigger Area Entity
		
	protected bool 										m_bHasItemSpawned 						= false;
	protected bool 										m_bHasItemDespawned 						= false;
	protected bool										m_bHasItemRestockEnded;
	
	protected CE_ItemData 								m_ItemSpawned;
	
	//protected ref CE_ItemDataConfig						m_Config;																	// config containing item data (typically in server profile folder)

	//------------------------------------------------------------------------------------------------
	void CE_ItemSpawningComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		ConnectToLootSpawningSystem();
		
		// Defaults for if no info gets set
		if (!m_Tier)
			m_Tier = CE_ELootTier.TIER1;
		
		if (!m_Usage)
			m_Usage = CE_ELootUsage.TOWN;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToLootSpawningSystem()
	{
		CE_ItemSpawningSystem updateSystem = CE_ItemSpawningSystem.GetInstance();
		
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
		
		if (m_sPrefab)
		{
			return;
		}
		else if (m_ItemDataConfig)
		{
			//m_Config = m_ItemDataConfig;
			
			GetItemToSpawn(m_ItemDataConfig);
		}
		else
		{
			if(GetGame().InPlayMode())
			{
				CE_WorldValidationComponent m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
				
				if (m_WorldValidationComponent)
				{
					if(m_WorldValidationComponent.GetWorldProcessed())
					{	
						//m_Config = m_WorldValidationComponent.GetItemDataConfig();
						
						GetItemToSpawn(m_WorldValidationComponent.GetItemDataConfig());
					}
					else
					{
						Print("[CentralEconomy] CE_WorldValidationComponent HAS NOT BEEN PROCESSED!", LogLevel.ERROR);
						return;
					}
				}
				else
				{
					Print("[CentralEconomy] YOU'RE MISSING CE_WorldValidationComponent IN YOUR GAMEMODE!", LogLevel.ERROR);
					return;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromLootSpawningSystem()
	{
		CE_ItemSpawningSystem updateSystem = CE_ItemSpawningSystem.GetInstance();
		
		if (!updateSystem)
			return;

		updateSystem.Unregister(this);
	}
	
	CE_ItemData GetItemToSpawn(CE_ItemDataConfig config)
	{
		CE_ItemSpawningSystem updateSystem = CE_ItemSpawningSystem.GetInstance();
		
		if (!updateSystem)
			return null;

		return updateSystem.GetItem(config, m_Tier, m_Usage);
	}
	
	
	
	
	
	
	
	
	
	
	
	/*
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
	*/
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
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
}
