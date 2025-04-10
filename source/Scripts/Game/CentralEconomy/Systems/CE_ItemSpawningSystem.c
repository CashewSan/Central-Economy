class CE_ItemSpawningSystem : GameSystem
{
	// To preface, this is a mess, but I'll try my best to keep comments for those reading lol
	
	protected ref array<ref CE_ItemData>				m_aSpawnedItems 				= new array<ref CE_ItemData>; 					// items that have spawned in the world
	protected ref array<CE_ItemSpawningComponent> 		m_aComponents 				= new array<CE_ItemSpawningComponent>; 			// initial pull of ALL item spawning components in the world
	protected ref map<ref CE_ItemData, string>			m_aItemsNotRestockReady 		= new map<ref CE_ItemData, string>;				// items whose restocking timer has not ended
	
	protected CE_WorldValidationComponent 				m_WorldValidationComponent;													// component added to world's gamemode for verification
	protected ref CE_ItemDataConfig					m_Config;																	// config containing item data (typically in server profile folder)
	protected CE_ELootTier							m_currentSpawnTier;															// current spawn tier that items are spawning at
	protected ref RandomGenerator 					m_randomGen 					= new RandomGenerator();							// vanilla random generator

	protected bool 									m_bWorldProcessed			= false;											// has the world been processed?
	
	protected float 									m_fTimer						= 0;												// timer for check interval
	
	protected int									CHECK_INTERVAL				= 0; 											// how often the item spawning system will run (in seconds)
	protected int 									m_iSpawningCount 			= 0;												// count for spawning limit
	
	//------------------------------------------------------------------------------------------------
	override void OnInit()
	{
		if (m_aComponents.IsEmpty())
			Enable(false);
		
		CHECK_INTERVAL = 20;
		
		//SetCurrentSpawnTier(CE_ELootTier.TIER1);
		
		DelayedInit();
	}
	
	//------------------------------------------------------------------------------------------------
	override event protected void OnUpdate(ESystemPoint point)
	{
		float timeSlice = GetWorld().GetFixedTimeSlice();

		m_fTimer += timeSlice;

		if (m_fTimer < CHECK_INTERVAL)
			return;

		m_fTimer = 0;
		
		if (m_bWorldProcessed)
		{
			SetNextSpawnTier();
			
			TryToSpawnItems();
		}
		else
			GetGame().GetCallqueue().CallLater(DelayedInit, 100, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedInit()
	{
		if(GetGame().InPlayMode())
			m_WorldValidationComponent = CE_WorldValidationComponent.GetInstance();
		
		if(m_WorldValidationComponent)
		{
			if(m_WorldValidationComponent.GetWorldProcessed())
			{	
				m_bWorldProcessed = true;
				
				SetNextSpawnTier();
				
				TryToSpawnItems();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void TryToSpawnItems()
	{
		//Print("Spawner Count: " + m_aComponents.Count());
		
		int itemCount = 0; // count for item limit
		int itemLimit = 20; // process only up to this amount of items at a time (helps performance)
		int spawningLimit = 30; // process only up to this amount of spawning at a time (helps performance)
		// so essentially, it'll process 20 items per 0.5 seconds 30 times. Potential for up to 600 items spawning in 15 seconds
		
		foreach (CE_ItemSpawningComponent comp : m_aComponents)
		{
			if (itemCount >= itemLimit)
			{
				if (m_iSpawningCount <= spawningLimit)
				{
					GetGame().GetCallqueue().CallLater(TryToSpawnItems, 500);
					m_iSpawningCount++;
				}
				else
				{
					//Print("SPAWNING LIMIT REACHED");
					m_iSpawningCount = 0;
				}
				
				//Print("ITEM LIMIT REACHED");
				break;
			}
			
			if (comp.HasItemSpawned())
			{		
				continue;
			}
			else if (comp.GetSpawnerTier() == GetCurrentSpawnTier())
			{
				comp.RequestItemToSpawn();
				
				itemCount++;
			}
		}
	}
	
	// GameSystem stuff
	//------------------------------------------------------------------------------------------------
	static CE_ItemSpawningSystem GetInstance()
	{
		World world = GetGame().GetWorld();

		if (!world)
			return null;

		return CE_ItemSpawningSystem.Cast(world.FindSystem(CE_ItemSpawningSystem));
	}

	//------------------------------------------------------------------------------------------------
	void Register(notnull CE_ItemSpawningComponent component)
	{
		if (!IsEnabled())
			Enable(true);
		
		if (!m_aComponents.Contains(component))
			m_aComponents.Insert(component);
	}

	//------------------------------------------------------------------------------------------------
	void Unregister(notnull CE_ItemSpawningComponent component)
	{
		m_aComponents.RemoveItem(component);
		
		Enable(false);
	}
	
	//------------------------------------------------------------------------------------------------
	CE_ItemData GetItem(CE_ItemDataConfig config, CE_ELootTier tier, CE_ELootUsage usage)
	{
		CE_ItemData itemChosen;
		
		if (!config || !config.m_ItemData || config.m_ItemData.IsEmpty())
			return null;
		
		array<ref CE_Item> items = GenerateItems(config.m_ItemData, tier, usage);
		
		if (items.IsEmpty())
			itemChosen = null;
		else
			itemChosen = SelectItem(items);
		
		/*
		if (itemChosen)
			Print("ItemChosen: " + itemChosen.m_sName);
		*/
		
		return itemChosen;
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	protected array<CE_ItemData> ValidateItemData(array<ref CE_ItemData> items)
	{
		array<CE_ItemData> itemData = {};
		
		if (!items || items && items.IsEmpty())
		{
			//GetGame().GetCallqueue().CallLater(ValidateItemData, 100, false, items); // pointless..?
			return null;
		}
		
		foreach(CE_ItemData item: items)
		{
			// basically, if missing any variable besides rotation or quantity min and max
			if(!item.m_sName || !item.m_sPrefab || !item.m_iNominal || !item.m_iMinimum || !item.m_iLifetime || !item.m_iRestock || !item.m_ItemCategory || !item.m_ItemUsages || !item.m_ItemTiers)
			{
				Print("[CentralEconomy] " + item.m_sName + " is missing information!", LogLevel.ERROR);
				continue;
			}
			else
				itemData.Insert(item);
		}
		
		return itemData;
	}
	*/
	
	//------------------------------------------------------------------------------------------------
	protected array<ref CE_Item> GenerateItems(array<ref CE_ItemData> items, CE_ELootTier tier, CE_ELootUsage usage)
	{
		array<ref CE_Item> itemsToSpawn = {};
		
		foreach (CE_ItemData itemData : items)
		{
			// basically, if missing any variable besides rotation or quantity min and max
			if(!itemData.m_sName 
			|| !itemData.m_sPrefab 
			|| !itemData.m_iNominal 
			|| !itemData.m_iMinimum 
			|| !itemData.m_iLifetime 
			|| !itemData.m_iRestock 
			|| !itemData.m_ItemCategory 
			|| !itemData.m_ItemUsages 
			|| !itemData.m_ItemTiers)
			{
				Print("[CentralEconomy] " + itemData.m_sName + " is missing information!", LogLevel.ERROR);
				continue;
			}
			else if (itemData.m_ItemTiers & tier 
			&& itemData.m_ItemUsages & usage 
			&& itemData.m_ItemTiers & GetCurrentSpawnTier()) // matches tier & usage of spawner component, as well as current tier being processed
			{
				int itemTargetCount = DetermineItemTargetCount(itemData, itemData.m_iMinimum, itemData.m_iNominal);
				
				if (itemTargetCount != 0)
				{
					CE_Item item = new CE_Item(itemData, itemData.m_ItemTiers, itemData.m_ItemUsages, itemData.m_ItemCategory);
					
					if (!itemsToSpawn.Contains(item))
					{
						for (int i = 0; i < Math.ClampInt(itemTargetCount, 0, itemData.m_iNominal); i++)
						{
							itemsToSpawn.Insert(item);
						}
					}
				}
			}
		}
		
		return itemsToSpawn;
	}
	
	//------------------------------------------------------------------------------------------------
	protected int DetermineItemTargetCount(CE_ItemData item, int minimum, int nominal)
	{
		map<ref CE_ItemData, string> itemsNotRestockReady = GetItemsNotRestockReady();
		array<ref CE_ItemData> spawnedItems = GetSpawnedItems();
		
		int itemCount = 0;
		
		// for every item spawned in the world
		foreach (CE_ItemData spawnedItem : spawnedItems)
		{
			string name = spawnedItem.m_sName;
			
			// increase item count if there is a name match
			if (name == item.m_sName)
			{
				itemCount = itemCount + 1;
			}
		}
		
		int targetCount = Math.ClampInt(nominal - itemCount, 0, nominal);
		
		for (int i = 0; i <= itemsNotRestockReady.Count(); i++)
		{
			if (itemsNotRestockReady.GetKeyByValue(item.m_sName))
				targetCount = Math.ClampInt(targetCount - 1, 0, nominal);
		}
		
		//Print("Item: " + item.m_sName + "MaxTargetCount: " + targetCount);
		
		//int minTargetCount = Math.ClampInt(minimum - itemCount, 0, nominal);
		
		//Print("Item: " + item.m_sName + ", TargetCount: " + Math.ClampInt(targetCount, 0, nominal));
		
		return Math.ClampInt(targetCount, 0, nominal);
	}
	
	//------------------------------------------------------------------------------------------------
	protected CE_ItemData SelectItem(array<ref CE_Item> itemArray)
	{
		if(itemArray.IsEmpty())
			return null;
		
		CE_ItemData itemData;
		
		//Print(itemArray.Count());
		
		map<CE_Item, int> counts = new map<CE_Item, int>;
		
		int probabilityTotal = 0;
		
		for (int i = 0; i < itemArray.Count(); i++)
		{
			if (itemArray.Contains(itemArray[i]))
			{
				counts[itemArray[i]] = counts[itemArray[i]] + 1;
			}
			else
			{	
				counts[itemArray[i]] = 1;
			}
			
			probabilityTotal += counts.Get(itemArray[i]);
		}
		
		foreach (CE_Item item : itemArray)
		{
			if (Probability(counts.Get(item), probabilityTotal))
			{
				itemData = item.Item;
				break;
			}
		}
		
		return itemData;
    }
	
	//------------------------------------------------------------------------------------------------
	protected bool Probability(float probability, float probabilityTotal)
	{
		int random = m_randomGen.RandInt(0, probabilityTotal);
		
		return random <= probability;
	}
	
	// Getters, Setters
	//------------------------------------------------------------------------------------------------
	CE_ELootTier GetCurrentSpawnTier()
	{
		return m_currentSpawnTier;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCurrentSpawnTier(CE_ELootTier tier)
	{
		m_currentSpawnTier = tier;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetNextSpawnTier()
	{	
		switch (GetCurrentSpawnTier())
		{
			case null:
				SetCurrentSpawnTier(CE_ELootTier.TIER1);
				break;
		
			case CE_ELootTier.TIER1:
				SetCurrentSpawnTier(CE_ELootTier.TIER2);
				break;
			
			case CE_ELootTier.TIER2:
				SetCurrentSpawnTier(CE_ELootTier.TIER3);
				break;
			
			case CE_ELootTier.TIER3:
				SetCurrentSpawnTier(CE_ELootTier.TIER4);
				break;
			
			case CE_ELootTier.TIER4:
				SetCurrentSpawnTier(CE_ELootTier.TIER1);
				if (CHECK_INTERVAL != 120)
					CHECK_INTERVAL = 120;
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref CE_ItemData> GetSpawnedItems()
	{
		return m_aSpawnedItems;
	}
	
	//------------------------------------------------------------------------------------------------
	map<ref CE_ItemData, string> GetItemsNotRestockReady()
	{
		return m_aItemsNotRestockReady;
	}
}





class CE_Item
{
	CE_ItemData Item;
	CE_ELootTier Tiers;
	CE_ELootUsage Usages;
	CE_ELootCategory Category;
	
	//------------------------------------------------------------------------------------------------
	void CE_Item(CE_ItemData item, CE_ELootTier tiers, CE_ELootUsage usages, CE_ELootCategory category)
	{
		Item = item;
		Tiers = tiers;
		Usages = usages;
		Category = category;
	}
}

class CE_Spawn
{
	CE_ItemSpawningComponent Spawn;
	CE_ELootTier Tier;
	CE_ELootUsage Usage;
	CE_ELootCategory Category;
	ref array<CE_Item> Items = {};
	
	//------------------------------------------------------------------------------------------------
	void CE_Spawn(CE_ItemSpawningComponent spawn, CE_ELootTier tier, CE_ELootUsage usage, CE_ELootCategory category)
	{
		Spawn = spawn;
		Tier = tier;
		Usage = usage;
		Category = category;
		Items = new array<CE_Item>;
	}
}