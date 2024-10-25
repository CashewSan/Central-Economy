class CE_ItemSpawningSystem : GameSystem
{
	private const int							lootSystemRefresh			= 5000; 									// how often the item spawning system will run, set to 5000 (5 seconds)
	
	protected ref array<CE_ItemData>				m_aItems 					= new array<CE_ItemData>; 				// initial pull of ALL item data directly from config
	protected ref array<ref CE_Items>				m_aItemsToSpawn 				= new array<ref CE_Items>; 				// item data containing items that need to spawn
	protected ref array<ref CE_ItemData>			m_aSpawnedItems 				= new array<ref CE_ItemData>; 			// items that have spawned in the world
	protected ref array<CE_ItemSpawningComponent> 	m_aComponents 				= new array<CE_ItemSpawningComponent>; 	// initial pull of ALL item spawning components in the world
	protected ref array<ref CE_Spawns> 			m_aComponentsForSpawn 		= new array<ref CE_Spawns>;  				// item spawning components in the world WITH NO LOOT
	protected ref array<CE_Spawns> 				m_aMatchedSpawns 			= new array<CE_Spawns>;					// spawns matched to item array
	protected ref array<ref CE_ItemData>			m_aItemsNotRestocked 			= new array<ref CE_ItemData>;				// items whose restocking timer has not ended
	
	protected CE_WorldValidationComponent 			m_WorldValidationComponent;											// component added to world's gamemode for verification
	protected ref CE_LootSpawningConfig 			m_Config;															// configs pulled from item spawning components
	
	protected bool 								b_HasGeneratedSpawnData		= false;
	protected bool 								b_HasGeneratedItemData		= false;
	
	protected ref RandomGenerator 				randomGen 					= new RandomGenerator();
	
	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{
		Init();
		GetGame().GetCallqueue().CallLater(Init, lootSystemRefresh, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		if(GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();

		if(m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.GetWorldProcessed())
			{	
				GenerateSpawnData();
				GenerateItemData();
				TryToSpawnLoot();
			}
			else
			{
				Print("[Central Economy] CE_WorldValidationComponent HAS NOT BEEN PROCESSED!", LogLevel.ERROR);
				return;
			}
		}
		else
		{
			Print("[Central Economy] YOU'RE MISSING CE_WorldValidationComponent IN YOUR GAMEMODE!", LogLevel.ERROR);
			return;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateSpawnData()
	{
		if (m_aComponents.Count() == 0)
			return;
		
		m_aComponentsForSpawn.Clear();
		
		foreach(CE_ItemSpawningComponent comp : m_aComponents)
		{
			foreach(ResourceName m_ResourceName : comp.m_sConfigs)
			{
				if(!m_ResourceName)
					continue;
				
				Resource m_Resource = BaseContainerTools.LoadContainer(m_ResourceName);
				BaseContainer m_Container = m_Resource.GetResource().ToBaseContainer();
				
				m_Config = CE_LootSpawningConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(m_Container));
			}
			
			CE_Spawns spawns = new CE_Spawns(comp, comp.m_Tier, comp.m_Usage, comp.m_Category);
			
			if (!comp.GetHasItemSpawned())
			{
				m_aComponentsForSpawn.Insert(spawns);
			}
			/*
			else if (comp.GetHasItemSpawned()) // not really needed because we clear the array?
			{
				m_aComponentsForSpawn.RemoveItem(spawns);
			}
			*/
		}
		
		SetHasGeneratedSpawnData(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHasGeneratedSpawnData(bool meow)
	{
		b_HasGeneratedSpawnData = meow;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasGeneratedSpawnData()
	{
		return b_HasGeneratedSpawnData;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_Spawns> GetGeneratedSpawnData()
	{
		return m_aComponentsForSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void GenerateItemData()
	{
		if(!m_Config)
			return;
		
		if (m_Config.m_ItemData.Count() == 0)
			return;
		
		m_aItemsToSpawn.Clear();
		
		foreach(CE_ItemData item: m_Config.m_ItemData)
		{
			if(!item.m_sName || !item.m_sPrefab)
				continue;
			
			int itemTargetCount = DetermineItemTargetCount(item, item.m_iMinimum, item.m_iNominal);
			
			//Print("" + item.m_sName + " " + itemTargetCount);
			
			array<CE_ELootTier> tiers = {item.m_ItemTiers};
			array<CE_ELootUsage> usages = {item.m_ItemUsages};
			
			CE_Items items = new CE_Items(item, tiers, usages, item.m_ItemCategory);
			
			if (!m_aItemsToSpawn.Contains(items))
			{
				for (int i = 0; i < itemTargetCount; i++)
				{
					m_aItemsToSpawn.Insert(items);
				}
			}
		}
		
		//Print("Items To Spawn: " + m_aItemsToSpawn.Count());
		
		SetHasGeneratedItemData(true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetHasGeneratedItemData(bool meow)
	{
		b_HasGeneratedItemData = meow;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetHasGeneratedItemData()
	{
		return b_HasGeneratedItemData;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_Items> GetGeneratedItemData()
	{
		return m_aItemsToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int DetermineItemTargetCount(CE_ItemData item, int minimum, int nominal)
	{
		// CONSIDER RESTOCK, FOR EACH ITEM THAT HAS NOT MET RESTOCK BOOL TRUE, DO -1 FROM TARGET COUNT
		
		int itemTargetCount;
		int itemsBeingRestocked = 0;
		
		if (GetSpawnedItems().Count() > 0)
		{
			int itemCount = 0;
			
			foreach (CE_ItemData spawnedItem : GetSpawnedItems())
			{
				string name = spawnedItem.m_sName;
				
				if (name == item.m_sName)
				{
					itemCount = itemCount + 1;
				}
			}
			
			int spawnCount = DetermineAvailableSpawnCount(item);
			
			if (itemCount >= nominal || spawnCount == 0)
			{
				itemTargetCount = 0; // no additional spawns of item needed
			}
			else
			{
				itemTargetCount = Math.ClampInt(itemCount + spawnCount, minimum, nominal);
			}
		}
		else
		{
			itemTargetCount = nominal;
		}
		
		
		if (GetItemsNotRestocked().Contains(item))
		{
			foreach (CE_ItemData itemNotRestocked : GetItemsNotRestocked())
			{
				itemsBeingRestocked = itemsBeingRestocked + 1;
			}
		}	
		
		if (itemTargetCount == 0)
		{
			return itemTargetCount;
		}
		else
		{
			return itemTargetCount - itemsBeingRestocked;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_ItemData> GetItemsNotRestocked()
	{
		return m_aItemsNotRestocked;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int DetermineAvailableSpawnCount(CE_ItemData item)
	{
		int availableSpawns;
		
		array<CE_ELootTier> tiers = {item.m_ItemTiers};
		array<CE_ELootUsage> usages = {item.m_ItemUsages};
		array<CE_Spawns> matchingSpawns = {};
		
		foreach (CE_Spawns spawns : m_aComponentsForSpawn)
		{
			array<int> intItemTiers = {};
			array<int> intItemUsages = {};
			
			foreach (CE_ELootTier tier : tiers)
			{
				SCR_Enum.BitToIntArray(tier, intItemTiers);
			}
			
			foreach (CE_ELootUsage usage : usages)
			{
				SCR_Enum.BitToIntArray(usage, intItemUsages);
			}
			
			if (intItemTiers.Contains(spawns.Tier) &&
			intItemUsages.Contains(spawns.Usage) &&
			item.m_ItemCategory == spawns.Category)
			{
				matchingSpawns.Insert(spawns);
			}
		}
		availableSpawns = matchingSpawns.Count();
		
		return availableSpawns;
	}
	
	//------------------------------------------------------------------------------------------------
	protected array<CE_Spawns> DetermineItemPool()
	{
		array<CE_Spawns> matchingSpawns = new array<CE_Spawns>;
		array<CE_Items> matchingItems = new array<CE_Items>;
		
		foreach (CE_Items item : GetGeneratedItemData())
		{	
			array<int> intItemTiers = {};
			array<int> intItemUsages = {};
			
			foreach (CE_ELootTier tier : item.Tiers)
			{
				SCR_Enum.BitToIntArray(tier, intItemTiers);
			}
			
			foreach (CE_ELootUsage usage : item.Usages)
			{
				SCR_Enum.BitToIntArray(usage, intItemUsages);
			}
			
			foreach (CE_Spawns spawns : m_aComponentsForSpawn)
			{	
				//Print("MatchingItems: " + matchingItems.Count());
				
				if (intItemTiers.Contains(spawns.Tier) &&
				intItemUsages.Contains(spawns.Usage) &&
				item.Category == spawns.Category)
				{
					if (!matchingSpawns.Contains(spawns))
					{
						matchingSpawns.Insert(spawns);
					}
					
					if (!matchingItems.Contains(item))
					{
						matchingItems.Insert(item);
					}
				}
			}
		}

		if (matchingSpawns.Count() > 0 && matchingItems.Count() > 0)
		{
			foreach (CE_Items matchedItem : matchingItems)
			{
				array<int> intMatchedItemTiers = {};
				array<int> intMatchedItemUsages = {};
				
				foreach (CE_ELootTier tier : matchedItem.Tiers)
				{
					SCR_Enum.BitToIntArray(tier, intMatchedItemTiers);
				}
				
				foreach (CE_ELootUsage usage : matchedItem.Usages)
				{
					SCR_Enum.BitToIntArray(usage, intMatchedItemUsages);
				}
				
				foreach (CE_Spawns matchedSpawn : matchingSpawns)
				{
					// why I gotta check again, idk, but it works now
					if (intMatchedItemTiers.Contains(matchedSpawn.Tier) &&
					intMatchedItemUsages.Contains(matchedSpawn.Usage) &&
					matchedItem.Category == matchedSpawn.Category)
					{
						matchedSpawn.Items.Insert(matchedItem);
					}
				}
			}
		}
		
		return matchingSpawns;
	}
	
	//------------------------------------------------------------------------------------------------
	void TryToSpawnLoot()
	{
		if (!Replication.IsServer())
			return;
		
		//Print(m_aItemsToSpawn.Count());
		
		if (GetHasGeneratedSpawnData() == false || GetHasGeneratedItemData() == false)
			return;
		
		m_aMatchedSpawns.Clear(); // doesnt clear properly for some reason?
		
		m_aMatchedSpawns = DetermineItemPool();
		
		//Print("MatchedSpawns: " + m_aMatchedSpawns.Count());
		
		if (m_aMatchedSpawns.Count() == 0)
			return;
		
		CE_ItemData m_Item;
		Resource m_Resource;
		array<CE_ItemData> meow = {};
		
		foreach (CE_Spawns spawns : m_aMatchedSpawns)
		{
			CE_ItemSpawningComponent comp = spawns.Spawn;
			
			IEntity lootSpawn = comp.GetOwner();
		
			vector m_WorldTransform[4];
			lootSpawn.GetWorldTransform(m_WorldTransform); // defines World Transform vector
			
			if (!comp.GetHasItemSpawned())
			{			
				//Print("meow: " + spawns.Items.Count());
				
				if (!spawns.Items)
					continue;
				
				m_Item = GetRandomItem(spawns.Items);
				
				if(!m_Item)
					continue;
				
				//Print(m_Item);
				
				m_Resource = Resource.Load(m_Item.m_sPrefab);
				
				EntitySpawnParams params();
				m_WorldTransform[3][1] = m_WorldTransform[3][1] + 0.200;
				params.Transform = m_WorldTransform;
			
				IEntity newEnt = GetGame().SpawnEntityPrefab(m_Resource, GetGame().GetWorld(), params);
				
				newEnt.SetYawPitchRoll(m_Item.m_vItemRotation + lootSpawn.GetYawPitchRoll());
				SCR_EntityHelper.SnapToGround(newEnt);
				
				m_aSpawnedItems.Insert(m_Item);
				comp.SetItemSpawned(m_Item);
				lootSpawn.AddChild(newEnt, -1, EAddChildFlags.NONE);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected CE_ItemData GetRandomItem(array<CE_Items> itemArray)
	{
		if(itemArray.Count() == 0)
			return null;
		int count = itemArray.Count() - 1;
		int random = randomGen.RandInt(0, count);
		
		Print(itemArray.Count());
		
		CE_Items item = itemArray[random];
		
		return item.Item;
    }
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_ItemData> GetSpawnedItems()
	{
		return m_aSpawnedItems;
	}

	// GameSystem stuff
	//------------------------------------------------------------------------------------------------
	void Register(CE_ItemSpawningComponent component)
	{
		if (!m_aComponents.Contains(component))
			m_aComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(CE_ItemSpawningComponent component)
	{
		m_aComponents.RemoveItem(component);
	}
}

class CE_Items
{
	CE_ItemData Item;
	ref array<CE_ELootTier> Tiers = {};
	ref array<CE_ELootUsage> Usages = {};
	CE_ELootCategory Category;
	
	//------------------------------------------------------------------------------------------------
	void CE_Items(CE_ItemData item, array<CE_ELootTier> tiers, array<CE_ELootUsage> usages, CE_ELootCategory category)
	{
		Item = item;
		Tiers = tiers;
		Usages = usages;
		Category = category;
	}
}

class CE_Spawns
{
	CE_ItemSpawningComponent Spawn;
	CE_ELootTier Tier;
	CE_ELootUsage Usage;
	CE_ELootCategory Category;
	ref array<CE_Items> Items = {};
	
	//------------------------------------------------------------------------------------------------
	void CE_Spawns(CE_ItemSpawningComponent spawn, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		Spawn = spawn;
		Tier = tier;
		Usage = usage;
		Category = category;
		Items = new array<CE_Items>;
	}
}